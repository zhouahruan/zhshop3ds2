/**
 * HTTP wrapper built on libctru's httpc service.
 *
 * libcurl is avoided because the 3DS toolchain no longer ships the BSD
 * socket shim (libsocs) that libcurl's 3DS build depends on. httpc is the
 * native 3DS HTTP/HTTPS client and works directly over the SOC service.
 *
 * GET/POST block briefly on the calling thread. http_download streams the
 * body to a file with optional progress callbacks.
 */
#include "http.h"
#include "api_config.h"
#include "../core/utils.h"

#include <3ds.h>
#include <3ds/allocator/linear.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* httpc requires the request URL to live in a 0x1000-byte aligned,
 * physically-contiguous buffer. linearAlloc() guarantees this. */
static char* alloc_aligned_url(const char* url) {
    if (!url) return NULL;
    size_t len = strlen(url);
    size_t cap = ((len + 0x1000) / 0x1000) * 0x1000;
    char* buf = (char*)linearAlloc(cap);
    if (!buf) return NULL;
    memcpy(buf, url, len + 1);
    return buf;
}

void http_init(void) {
    /* httpcInit(0) is called from main.c. */
}

void http_exit(void) {
    /* httpcExit() is called from main.c. */
}

static void apply_default_headers(httpcContext* ctx) {
    httpcAddRequestHeaderField(ctx, "User-Agent", "3DSAppStore/1.0 (httpc)");
#ifdef USER_TOKEN
    if (USER_TOKEN[0] != '\0') {
        char auth[256];
        snprintf(auth, sizeof(auth), "Bearer %s", USER_TOKEN);
        httpcAddRequestHeaderField(ctx, "Authorization", auth);
    }
#endif
}

static void apply_ssl(httpcContext* ctx) {
    /* The 3DS trust store is incomplete for modern sites; skip peer
     * verification to reach the backend. */
    httpcSetSSLOpt(ctx, SSLCOPT_DisableVerify);
}

#define HTTP_TIMEOUT_NS ((u64)API_TIMEOUT_MS * 1000000ULL)

/* Read the full response body into a malloc'd buffer. Returns NULL on
 * failure. The buffer is NUL-terminated. */
static char* read_body(httpcContext* ctx, u32* out_len) {
    u32 content_size = 0;
    httpcGetDownloadSizeState(ctx, NULL, &content_size);

    u32 cap = content_size > 0 ? content_size + 1 : 256 * 1024;
    char* buf = (char*)malloc(cap);
    if (!buf) return NULL;

    u32 total = 0;
    while (1) {
        u32 before = 0;
        httpcGetDownloadSizeState(ctx, &before, NULL);

        Result rc = httpcReceiveDataTimeout(ctx, (u8*)(buf + total),
                                            (u32)(cap - 1 - total), HTTP_TIMEOUT_NS);
        if (R_FAILED(rc)) { free(buf); return NULL; }

        u32 after = 0;
        httpcGetDownloadSizeState(ctx, &after, NULL);
        u32 delta = after - before;
        if (delta == 0) break; /* EOF */

        total += delta;

        /* Grow buffer if we're running out of space. */
        if (total + 1 >= cap) {
            u32 new_cap = cap * 2;
            char* nb = (char*)realloc(buf, new_cap);
            if (!nb) { free(buf); return NULL; }
            buf = nb;
            cap = new_cap;
        }
    }

    buf[total] = '\0';
    if (out_len) *out_len = total;
    return buf;
}

NetStatus http_get(const char* url, NetResult* out) {
    if (!out) return NET_FAIL;
    out->data = NULL; out->len = 0; out->http_code = 0; out->status = NET_FAIL;

    char* aligned = alloc_aligned_url(url);
    if (!aligned) return NET_FAIL;

    httpcContext ctx;
    Result rc = httpcOpenContext(&ctx, HTTPC_METHOD_GET, aligned, 0);
    if (R_FAILED(rc)) {
        linearFree(aligned);
        return NET_FAIL;
    }

    apply_default_headers(&ctx);
    apply_ssl(&ctx);

    rc = httpcBeginRequest(&ctx);
    if (R_FAILED(rc)) {
        httpcCloseContext(&ctx);
        linearFree(aligned);
        return NET_FAIL;
    }

    u32 code = 0;
    rc = httpcGetResponseStatusCodeTimeout(&ctx, &code, HTTP_TIMEOUT_NS);
    if (R_FAILED(rc)) {
        httpcCloseContext(&ctx);
        linearFree(aligned);
        return NET_FAIL;
    }
    out->http_code = (int)code;

    u32 got = 0;
    char* buf = read_body(&ctx, &got);
    httpcCloseContext(&ctx);
    linearFree(aligned);
    if (!buf) return NET_FAIL;

    out->data = buf;
    out->len = got;
    out->status = (code >= 200 && code < 300) ? NET_OK : NET_FAIL;
    return out->status;
}

NetStatus http_post(const char* url, const char* body, NetResult* out) {
    if (!out) return NET_FAIL;
    out->data = NULL; out->len = 0; out->http_code = 0; out->status = NET_FAIL;

    char* aligned = alloc_aligned_url(url);
    if (!aligned) return NET_FAIL;

    httpcContext ctx;
    Result rc = httpcOpenContext(&ctx, HTTPC_METHOD_POST, aligned, 0);
    if (R_FAILED(rc)) {
        linearFree(aligned);
        return NET_FAIL;
    }

    apply_default_headers(&ctx);
    httpcAddRequestHeaderField(&ctx, "Content-Type", "application/json");
    apply_ssl(&ctx);

    void* pdata = NULL;
    if (body && body[0]) {
        size_t blen = strlen(body);
        /* httpcAddPostDataRaw expects u32-aligned data; copy to aligned buf. */
        pdata = linearAlloc((blen + 63) & ~63u);
        if (pdata) {
            memcpy(pdata, body, blen);
            httpcAddPostDataRaw(&ctx, (const u32*)pdata, (u32)blen);
        }
    }

    rc = httpcBeginRequest(&ctx);
    if (R_FAILED(rc)) {
        if (pdata) linearFree(pdata);
        httpcCloseContext(&ctx);
        linearFree(aligned);
        return NET_FAIL;
    }

    u32 code = 0;
    httpcGetResponseStatusCodeTimeout(&ctx, &code, HTTP_TIMEOUT_NS);
    out->http_code = (int)code;

    u32 got = 0;
    char* buf = read_body(&ctx, &got);
    if (pdata) linearFree(pdata);
    httpcCloseContext(&ctx);
    linearFree(aligned);
    if (!buf) return NET_FAIL;

    out->data = buf;
    out->len = got;
    out->status = (code >= 200 && code < 300) ? NET_OK : NET_FAIL;
    return out->status;
}

NetStatus http_download(const char* url, const char* save_path,
                        ProgressCb cb, void* user) {
    FILE* fp = fopen(save_path, "wb");
    if (!fp) return NET_FAIL;

    char* aligned = alloc_aligned_url(url);
    if (!aligned) { fclose(fp); return NET_FAIL; }

    httpcContext ctx;
    Result rc = httpcOpenContext(&ctx, HTTPC_METHOD_GET, aligned, 0);
    if (R_FAILED(rc)) {
        linearFree(aligned);
        fclose(fp);
        return NET_FAIL;
    }

    apply_default_headers(&ctx);
    apply_ssl(&ctx);

    rc = httpcBeginRequest(&ctx);
    if (R_FAILED(rc)) {
        httpcCloseContext(&ctx);
        linearFree(aligned);
        fclose(fp);
        return NET_FAIL;
    }

    u32 code = 0;
    httpcGetResponseStatusCodeTimeout(&ctx, &code, HTTP_TIMEOUT_NS);
    if (code < 200 || code >= 300) {
        httpcCloseContext(&ctx);
        linearFree(aligned);
        fclose(fp);
        return NET_FAIL;
    }

    u32 total = 0;
    httpcGetDownloadSizeState(&ctx, NULL, &total);

    /* Download in chunks. httpcReceiveData fills the buffer; the actual
     * byte count is the delta of the download-size state before/after. */
    u8* chunk = (u8*)linearAlloc(0x10000);  /* 64 KiB aligned buffer */
    if (!chunk) {
        httpcCloseContext(&ctx);
        linearFree(aligned);
        fclose(fp);
        return NET_FAIL;
    }

    u32 downloaded = 0;
    NetStatus status = NET_OK;
    while (1) {
        u32 before = 0;
        httpcGetDownloadSizeState(&ctx, &before, NULL);

        rc = httpcReceiveDataTimeout(&ctx, chunk, 0x10000, HTTP_TIMEOUT_NS);
        if (R_FAILED(rc)) { status = NET_FAIL; break; }

        u32 after = 0;
        httpcGetDownloadSizeState(&ctx, &after, NULL);
        u32 delta = after - before;
        if (delta == 0) break; /* EOF */
        if (delta > 0x10000) delta = 0x10000;

        fwrite(chunk, 1, delta, fp);
        downloaded = after;
        if (cb) cb(user, (int64_t)total, (int64_t)downloaded);
        if (total > 0 && downloaded >= total) break;
    }

    linearFree(chunk);
    httpcCloseContext(&ctx);
    linearFree(aligned);
    fclose(fp);
    return status;
}

void http_result_free(NetResult* r) {
    if (!r) return;
    if (r->data) free(r->data);
    r->data = NULL; r->len = 0;
}

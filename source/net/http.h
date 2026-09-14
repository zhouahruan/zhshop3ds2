/**
 * libcurl HTTP wrapper.
 *
 * GET/POST run on the main thread (block briefly); download streaming
 * writes to file with progress callbacks. For long downloads use the
 * downloader module which runs curl multi.
 */
#ifndef NET_HTTP_H
#define NET_HTTP_H

#include <3ds.h>
#include <stdint.h>
#include <stddef.h>

typedef enum { NET_OK, NET_FAIL, NET_TIMEOUT } NetStatus;

typedef struct {
    char*  data;        /* malloc'd response body, NULL on failure */
    size_t len;
    int    http_code;
    NetStatus status;
} NetResult;

typedef int (*ProgressCb)(void* user, int64_t dl_total, int64_t dl_now);

void http_init(void);
void http_exit(void);

/* Blocking GET. out->data must be free()'d by caller. */
NetStatus http_get(const char* url, NetResult* out);

/* Blocking POST with JSON body. */
NetStatus http_post(const char* url, const char* body, NetResult* out);

/* Streaming download to file path on SD. cb optional. */
NetStatus http_download(const char* url, const char* save_path,
                        ProgressCb cb, void* user);

/* Free the data pointer inside a NetResult. */
void http_result_free(NetResult* r);

#endif /* NET_HTTP_H */

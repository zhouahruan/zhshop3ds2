#include "api.h"
#include "api_config.h"
#include "http.h"
#include "json_parse.h"
#include "../core/utils.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#if USE_MOCK
#include "mock/mock.h"
#endif

/*---- helpers ----*/

static int get_json(const char* path, NetResult* r) {
    char url[512];
    snprintf(url, sizeof(url), "%s%s", API_BASE_URL, path);
    NetStatus s = http_get(url, r);
    if (s != NET_OK || !r->data) {
        http_result_free(r);
        return 0;
    }
    return 1;
}

static int post_json(const char* path, const char* body, NetResult* r) {
    char url[512];
    snprintf(url, sizeof(url), "%s%s", API_BASE_URL, path);
    NetStatus s = http_post(url, body, r);
    if (s != NET_OK) {
        http_result_free(r);
        return 0;
    }
    return 1;
}

/* Global endpoints (visitors/pulse/announcements/search) live at the
 * zhshop-api root without the /3ds zone prefix. */
static int get_global_json(const char* path, NetResult* r) {
    char url[512];
    snprintf(url, sizeof(url), "%s%s", API_GLOBAL_BASE_URL, path);
    NetStatus s = http_get(url, r);
    if (s != NET_OK || !r->data) {
        http_result_free(r);
        return 0;
    }
    return 1;
}

/*================= API METHODS =================*/

static int api_get_recommend(AppList* out) {
#if USE_MOCK
    return mock_get_recommend(out);
#else
    /* Backend has no /recommend; use top-by-downloads as the featured list. */
    NetResult r = {0};
    if (!get_json("/apps?sort=download_count&page_size=12", &r)) return 0;
    int ok = json_parse_app_list(r.data, out);
    http_result_free(&r);
    return ok;
#endif
}

static int api_get_categories(CategoryList* out) {
#if USE_MOCK
    return mock_get_categories(out);
#else
    NetResult r = {0};
    if (!get_json("/categories", &r)) return 0;
    int ok = json_parse_category_list(r.data, out);
    http_result_free(&r);
    return ok;
#endif
}

static int api_get_app_list(const char* category_id, const char* keyword,
                            const char* sort,
                            const char* region, int cfw_required,
                            const char* min_firmware,
                            int page, int page_size, AppList* out) {
#if USE_MOCK
    return mock_get_app_list(category_id, keyword, sort,
                             region, cfw_required, min_firmware,
                             page, page_size, out);
#else
    char path[700];
    char q[600] = "?";
    int first = 1;
    if (category_id && category_id[0]) {
        snprintf(q + strlen(q), sizeof(q) - strlen(q),
                 "%scategory_id=%s", first ? "" : "&", category_id);
        first = 0;
    }
    if (keyword && keyword[0]) {
        snprintf(q + strlen(q), sizeof(q) - strlen(q),
                 "%skeyword=%s", first ? "" : "&", keyword);
        first = 0;
    }
    if (region && region[0]) {
        snprintf(q + strlen(q), sizeof(q) - strlen(q),
                 "%sregion=%s", first ? "" : "&", region);
        first = 0;
    }
    if (cfw_required >= 0) {
        snprintf(q + strlen(q), sizeof(q) - strlen(q),
                 "%scfw_required=%s", first ? "" : "&",
                 cfw_required ? "true" : "false");
        first = 0;
    }
    if (min_firmware && min_firmware[0]) {
        snprintf(q + strlen(q), sizeof(q) - strlen(q),
                 "%smin_firmware=%s", first ? "" : "&", min_firmware);
        first = 0;
    }
    if (sort && sort[0]) {
        snprintf(q + strlen(q), sizeof(q) - strlen(q),
                 "%ssort=%s", first ? "" : "&", sort);
        first = 0;
    }
    if (first) {
        snprintf(q + strlen(q), sizeof(q) - strlen(q), "page=%d", page);
        first = 0;
    } else {
        snprintf(q + strlen(q), sizeof(q) - strlen(q), "&page=%d", page);
    }
    snprintf(q + strlen(q), sizeof(q) - strlen(q), "&page_size=%d",
             page_size > 0 ? page_size : 20);
    snprintf(path, sizeof(path), "/apps%s", q);
    NetResult r = {0};
    if (!get_json(path, &r)) return 0;
    int ok = json_parse_app_list(r.data, out);
    http_result_free(&r);
    return ok;
#endif
}

static int api_get_app_detail(const char* app_id, AppDetail* out) {
#if USE_MOCK
    return mock_get_app_detail(app_id, out);
#else
    char path[256];
    snprintf(path, sizeof(path), "/apps/%s", app_id ? app_id : "");
    NetResult r = {0};
    if (!get_json(path, &r)) return 0;
    int ok = json_parse_app_detail(r.data, out);
    http_result_free(&r);
    return ok;
#endif
}

static int api_get_download_url(const char* app_id, char* out_url, int url_cap,
                                int* out_size) {
#if USE_MOCK
    return mock_get_download_url(app_id, out_url, url_cap, out_size);
#else
    /* The download endpoint returns an SSE/octet-stream, not JSON.
     * Hand the download URL straight to the downloader; it uses libcurl
     * to stream the cia to disk. */
    if (!app_id || !out_url || url_cap <= 0) return 0;
    snprintf(out_url, (size_t)url_cap, "%s/apps/%s/download",
             API_BASE_URL, app_id);
    if (out_size) *out_size = 0;   /* known only after the stream starts */
    return 1;
#endif
}

static int api_get_post_list(int page, PostList* out) {
#if USE_MOCK
    return mock_get_post_list(page, out);
#else
    char path[128];
    snprintf(path, sizeof(path), "/forum/posts?page=%d", page);
    NetResult r = {0};
    if (!get_json(path, &r)) return 0;
    int ok = json_parse_post_list(r.data, out);
    http_result_free(&r);
    return ok;
#endif
}

static int api_get_post_detail(const char* post_id, Post* out, ReplyList* out_replies) {
#if USE_MOCK
    return mock_get_post_detail(post_id, out, out_replies);
#else
    char path[256];
    snprintf(path, sizeof(path), "/forum/posts/%s", post_id ? post_id : "");
    NetResult r = {0};
    if (!get_json(path, &r)) return 0;
    int ok = json_parse_post_detail(r.data, out, out_replies);
    http_result_free(&r);
    return ok;
#endif
}

static int api_create_post(const char* title, const char* body) {
#if USE_MOCK
    return mock_create_post(title, body);
#else
    char buf[2300];
    snprintf(buf, sizeof(buf), "{\"title\":\"%s\",\"body\":\"%s\"}", title, body);
    NetResult r = {0};
    if (!post_json("/forum/posts", buf, &r)) return 0;
    int ok = (r.http_code >= 200 && r.http_code < 300);
    http_result_free(&r);
    return ok;
#endif
}

static int api_create_reply(const char* post_id, const char* body) {
#if USE_MOCK
    return mock_create_reply(post_id, body);
#else
    char path[256], buf[1300];
    snprintf(path, sizeof(path), "/forum/posts/%s/replies", post_id ? post_id : "");
    snprintf(buf,  sizeof(buf),  "{\"body\":\"%s\"}", body);
    NetResult r = {0};
    if (!post_json(path, buf, &r)) return 0;
    int ok = (r.http_code >= 200 && r.http_code < 300);
    http_result_free(&r);
    return ok;
#endif
}

static int api_get_chat_sessions(SessionList* out) {
#if USE_MOCK
    return mock_get_chat_sessions(out);
#else
    NetResult r = {0};
    if (!get_json("/chat/sessions", &r)) return 0;
    int ok = json_parse_session_list(r.data, out);
    http_result_free(&r);
    return ok;
#endif
}

static int api_get_messages(const char* session_id, MessageList* out) {
#if USE_MOCK
    return mock_get_messages(session_id, out);
#else
    char path[256];
    snprintf(path, sizeof(path), "/chat/sessions/%s/messages", session_id ? session_id : "");
    NetResult r = {0};
    if (!get_json(path, &r)) return 0;
    int ok = json_parse_message_list(r.data, out);
    http_result_free(&r);
    return ok;
#endif
}

static int api_send_message(const char* session_id, const char* content) {
#if USE_MOCK
    return mock_send_message(session_id, content);
#else
    char path[256], buf[1100];
    snprintf(path, sizeof(path), "/chat/sessions/%s/messages", session_id ? session_id : "");
    snprintf(buf,  sizeof(buf),  "{\"content\":\"%s\"}", content);
    NetResult r = {0};
    if (!post_json(path, buf, &r)) return 0;
    int ok = (r.http_code >= 200 && r.http_code < 300);
    http_result_free(&r);
    return ok;
#endif
}

static int api_login(const char* username, const char* password, User* out) {
#if USE_MOCK
    return mock_login(username, password, out);
#else
    char buf[512];
    snprintf(buf, sizeof(buf), "{\"username\":\"%s\",\"password\":\"%s\"}",
             username, password);
    NetResult r = {0};
    if (!post_json("/auth/login", buf, &r)) return 0;
    int ok = json_parse_user(r.data, out);
    http_result_free(&r);
    return ok;
#endif
}

const ApiClient api = {
    .get_recommend     = api_get_recommend,
    .get_categories     = api_get_categories,
    .get_app_list       = api_get_app_list,
    .get_app_detail     = api_get_app_detail,
    .get_download_url   = api_get_download_url,
    .get_post_list     = api_get_post_list,
    .get_post_detail    = api_get_post_detail,
    .create_post        = api_create_post,
    .create_reply       = api_create_reply,
    .get_chat_sessions  = api_get_chat_sessions,
    .get_messages       = api_get_messages,
    .send_message       = api_send_message,
    .login              = api_login,
};

/**
 * API client. Stable interface — scenes call api.x() exclusively.
 *
 * Backend: https://backend.appmiaoda.com/.../zhshop-api/3ds (zone base).
 * Global endpoints (visitors/pulse/announcements/search) live at
 * .../zhshop-api (no /3ds).
 *
 * Zone endpoints:
 *   GET  /apps?category_id=&keyword=&region=&cfw_required=
 *        &min_firmware=&sort=&page=&page_size=
 *   GET  /apps/{id}                   -> AppDetail
 *   GET  /apps/{id}/download          -> SSE octet-stream (use URL directly)
 *   GET  /categories                  -> [Category]
 *   POST /apps                        (JWT) submit app
 *
 * Global endpoints:
 *   GET  /visitors  GET/POST  (visits counter)
 *   GET  /pulse                 (site stats)
 *   GET  /announcements         (announcement list)
 *   GET  /search?keyword=       (global search)
 */
#ifndef NET_API_H
#define NET_API_H

#include "../data/types.h"

typedef struct {
    int (*get_recommend)(AppList* out);
    int (*get_categories)(CategoryList* out);
    int (*get_app_list)(const char* category_id, const char* keyword,
                        const char* sort,
                        const char* region, int cfw_required,
                        const char* min_firmware,
                        int page, int page_size, AppList* out);
    int (*get_app_detail)(const char* app_id, AppDetail* out);
    /* Fills out_url with the /apps/{id}/download stream URL (no JSON parse). */
    int (*get_download_url)(const char* app_id, char* out_url, int url_cap,
                             int* out_size);
    int (*get_post_list)(int page, PostList* out);
    int (*get_post_detail)(const char* post_id, Post* out, ReplyList* out_replies);
    int (*create_post)(const char* title, const char* body);
    int (*create_reply)(const char* post_id, const char* body);
    int (*get_chat_sessions)(SessionList* out);
    int (*get_messages)(const char* session_id, MessageList* out);
    int (*send_message)(const char* session_id, const char* content);
    int (*login)(const char* username, const char* password, User* out);
} ApiClient;

extern const ApiClient api;

#endif /* NET_API_H */

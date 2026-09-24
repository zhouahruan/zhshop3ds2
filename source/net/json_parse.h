/**
 * JSON parse helpers built on jansson.
 *
 * Each function takes the raw response body (as received by http_get/
 * http_post) and fills an output struct. Caller owns the struct; any
 * embedded arrays inside the struct are malloc'd and must be free'd via
 * the matching *_free function (defined alongside the parse function).
 */
#ifndef NET_JSON_PARSE_H
#define NET_JSON_PARSE_H

#include "../data/types.h"

/* Parse the JSON shape:
 *   { "items": [ {...}, ... ], "total": int, "page": int }
 * out->items is malloc'd. */
int json_parse_app_list(const char* body, AppList* out);
int json_parse_category_list(const char* body, CategoryList* out);
int json_parse_app_detail(const char* body, AppDetail* out);
int json_parse_download_info(const char* body, char* out_url, int url_cap,
                             int* out_size);

int json_parse_post_list(const char* body, PostList* out);
int json_parse_post_detail(const char* body, Post* out_post, ReplyList* out_replies);
int json_parse_session_list(const char* body, SessionList* out);
int json_parse_message_list(const char* body, MessageList* out);

int json_parse_user(const char* body, User* out);

void json_free_app_list(AppList* p);
void json_free_category_list(CategoryList* p);
void json_free_post_list(PostList* p);
void json_free_reply_list(ReplyList* p);
void json_free_session_list(SessionList* p);
void json_free_message_list(MessageList* p);

#endif /* NET_JSON_PARSE_H */

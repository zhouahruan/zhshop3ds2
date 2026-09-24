/**
 * Mock data getters. Declared here so api.c can call them when
 * USE_MOCK is 1. Each mock file (.mock.c) defines one or more.
 */
#ifndef NET_MOCK_MOCK_H
#define NET_MOCK_MOCK_H

#include "../api.h"

int mock_get_recommend(AppList* out);
int mock_get_categories(CategoryList* out);
int mock_get_app_list(const char* category_id, const char* keyword,
                      const char* sort,
                      const char* region, int cfw_required,
                      const char* min_firmware,
                      int page, int page_size, AppList* out);
int mock_get_app_detail(const char* app_id, AppDetail* out);
int mock_get_download_url(const char* app_id, char* out_url, int url_cap,
                           int* out_size);

int mock_get_post_list(int page, PostList* out);
int mock_get_post_detail(const char* post_id, Post* out, ReplyList* out_replies);
int mock_create_post(const char* title, const char* body);
int mock_create_reply(const char* post_id, const char* body);

int mock_get_chat_sessions(SessionList* out);
int mock_get_messages(const char* session_id, MessageList* out);
int mock_send_message(const char* session_id, const char* content);

int mock_login(const char* username, const char* password, User* out);

#endif /* NET_MOCK_MOCK_H */

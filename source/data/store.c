#include "store.h"
#include "../core/utils.h"
#include "../net/json_parse.h"

#include <stdlib.h>
#include <string.h>

AppStore g_store;

void store_init(void) {
    memset(&g_store, 0, sizeof(g_store));
    utils_strlcpy(g_store.list_sort, "hot", sizeof(g_store.list_sort));
    utils_strlcpy(g_store.current_route, "splash", sizeof(g_store.current_route));
}

void store_reset(void) {
    store_free_lists();
    store_init();
}

void store_set_route(const char* name) {
    if (!name) return;
    utils_strlcpy(g_store.current_route, name, sizeof(g_store.current_route));
}

void store_set_error(const char* msg) {
    if (!msg) { g_store.last_error[0] = '\0'; return; }
    utils_strlcpy(g_store.last_error, msg, sizeof(g_store.last_error));
}

void store_clear_error(void) {
    g_store.last_error[0] = '\0';
}

void store_free_lists(void) {
    json_free_app_list(&g_store.recommend);
    json_free_category_list(&g_store.categories);
    json_free_app_list(&g_store.current_list);
    json_free_post_list(&g_store.posts);
    json_free_reply_list(&g_store.replies);
    json_free_session_list(&g_store.sessions);
    json_free_message_list(&g_store.messages);
    if (g_store.current_detail) {
        free(g_store.current_detail);
        g_store.current_detail = NULL;
    }
}

int store_add_download(const App* app) {
    if (!app || g_store.download_count >= MAX_DOWNLOADS) return -1;
    int idx = g_store.download_count++;
    DownloadTask* t = &g_store.downloads[idx];
    memset(t, 0, sizeof(*t));
    utils_strlcpy(t->app_id,   app->id,   sizeof(t->app_id));
    utils_strlcpy(t->app_name, app->name, sizeof(t->app_name));
    t->total_bytes = app->package_size;
    t->status      = DL_PENDING;
    return idx;
}

int store_find_download(const char* app_id) {
    if (!app_id) return -1;
    for (int i = 0; i < g_store.download_count; ++i) {
        if (strcmp(g_store.downloads[i].app_id, app_id) == 0) return i;
    }
    return -1;
}

void store_remove_download(int idx) {
    if (idx < 0 || idx >= g_store.download_count) return;
    /* Shift remaining down. */
    for (int i = idx; i < g_store.download_count - 1; ++i) {
        g_store.downloads[i] = g_store.downloads[i + 1];
    }
    g_store.download_count--;
}

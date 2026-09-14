/**
 * Global domain store. Mirrors the React/zustand store from the
 * original plan but in C — a single static struct accessed via g_store.
 *
 * All scenes mutate this state. Reads are direct field access. For
 * long-living allocations (AppList items etc) owners must free before
 * re-fetching.
 */
#ifndef DATA_STORE_H
#define DATA_STORE_H

#include "types.h"

#define MAX_DOWNLOADS 16
#define MAX_INSTALLED  64
#define MAX_CATEGORIES 16

typedef struct {
    /* App list domain. */
    AppList  recommend;
    CategoryList categories;
    AppList  current_list;
    char     list_keyword[128];
    char     list_category[32];
    char     list_sort[16];           /* "hot" | "new" | "rating" */
    int      list_page;

    AppDetail* current_detail;        /* malloc'd; NULL when none */

    /* Downloads + installed. */
    DownloadTask downloads[MAX_DOWNLOADS];
    int          download_count;
    App          installed[MAX_INSTALLED];
    int          installed_count;

    /* Forum. */
    PostList posts;
    Post     current_post;
    ReplyList replies;

    /* Chat. */
    SessionList sessions;
    char       current_session_id[32];
    MessageList messages;

    /* User. */
    User user;

    /* UI. */
    char  current_route[32];
    char  last_error[128];
    uint8_t loading;
} AppStore;

extern AppStore g_store;

void store_init(void);
void store_reset(void);

/* Convenience setters. */
void store_set_route(const char* name);
void store_set_error(const char* msg);
void store_clear_error(void);

/* Free heap allocations under each list. */
void store_free_lists(void);

/* Download task management. */
int  store_add_download(const App* app);
int  store_find_download(const char* app_id);
void store_remove_download(int idx);

#endif /* DATA_STORE_H */

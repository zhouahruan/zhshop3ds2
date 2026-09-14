/**
 * Domain data types. All structs use fixed-size char buffers so the
 * app does not need a heap string allocator on hot paths.
 */
#ifndef DATA_TYPES_H
#define DATA_TYPES_H

#include <3ds.h>
#include <stdint.h>

#define APP_ID_LEN      32
#define APP_NAME_LEN    128
#define APP_DESC_LEN    1024
#define URL_LEN         256
#define SCREENSHOT_MAX  6

typedef struct {
    char id[APP_ID_LEN];
    char slug[96];
    char name[APP_NAME_LEN];
    char version[24];
    char short_desc[128];
    char description[APP_DESC_LEN];
    char icon_url[URL_LEN];
    char screenshot_urls[SCREENSHOT_MAX][URL_LEN];
    int  screenshot_count;
    float avg_rating;        /* 0.0 - 5.0 */
    int  rating_count;
    int  download_count;
    int  package_size;       /* bytes, 0 if unknown */
    char category_id[32];
    char category_name[64];
    /* 3DS-specific. */
    char region[16];         /* USA | EUR | JPN | KOR | CHN | Region Free */
    char min_firmware[32];   /* e.g. 11.16.0-48 */
    uint8_t cfw_required;
    char title_id[20];       /* 16 hex chars */
    char file_format[8];     /* .cia | .3ds | .cxi */
    uint8_t new3ds_exclusive;
    char app_source[16];     /* opensource | ... */
    char developer_name[64];
    char license_type[24];   /* freeware | ... */
    char created_at[32];
    char updated_at[32];
    char version_updated_at[32];
} App;

typedef struct { App* items; int count; int total; int page; int page_size; } AppList;

typedef struct {
    char id[32];
    char zone_id[32];
    char name[64];
    int  sort_order;
} Category;

typedef struct { Category* items; int count; } CategoryList;

typedef struct {
    App base;
    char long_description[APP_DESC_LEN * 2];
    char changelog[APP_DESC_LEN];
    char package_url[URL_LEN];
    char developer_website[URL_LEN];
    char developer_email[128];
    uint8_t has_ads;
    uint8_t requires_root;
    char permissions[256];
    char zone_name[32];
    char uploader_username[64];
    char uploader_avatar[URL_LEN];
    uint8_t uploader_certified;
} AppDetail;

typedef struct {
    char id[32];
    char title[128];
    char author[64];
    char body[2048];
    int  reply_count;
    u64  created_at;
} Post;

typedef struct { Post* items; int count; int total; int page; } PostList;

typedef struct {
    char id[32];
    char author[64];
    char body[1024];
    u64  created_at;
} Reply;

typedef struct { Reply* items; int count; } ReplyList;

typedef struct {
    char id[32];
    char session_id[32];
    char display_name[64];
    char last_message[128];
    u64  updated_at;
    int  unread;
} ChatSession;

typedef struct { ChatSession* items; int count; } SessionList;

typedef struct {
    char id[32];
    char content[1024];
    int  from_self;        /* 1 = me, 0 = other */
    u64  timestamp;
} Message;

typedef struct { Message* items; int count; } MessageList;

typedef enum { DL_PENDING, DL_RUNNING, DL_PAUSED, DL_DONE, DL_FAILED } DlStatus;

typedef struct {
    char app_id[APP_ID_LEN];
    char app_name[APP_NAME_LEN];
    char download_url[URL_LEN];
    DlStatus status;
    int progress;            /* 0-100 */
    int total_bytes;
    int downloaded_bytes;
    u64  started_at;
} DownloadTask;

typedef struct {
    int user_id;
    char username[64];
    char token[128];
    uint8_t is_login;
} User;

#endif /* DATA_TYPES_H */

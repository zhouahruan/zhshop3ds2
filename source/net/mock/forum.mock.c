/**
 * Mock forum data (posts + replies).
 */
#include "../api.h"

#include <string.h>

static Post s_posts[] = {
    {
        .id="post-1", .title="Welcome to 3DS App Store!",
        .author="admin", .body="Discuss releases, ask for ports, share CIA uploads here.",
        .reply_count=2, .created_at=1700000000000ULL
    },
    {
        .id="post-2", .title="Best GBA emulator?",
        .author="retrofan", .body="Looking for suggestions — open source please.",
        .reply_count=0, .created_at=1700000060000ULL
    },
    {
        .id="post-3", .title="FTPClient v2 out now",
        .author="ftp_dev", .body="Adds resume support + drag-drop. Try it!",
        .reply_count=0, .created_at=1700000120000ULL
    },
};
static const int s_posts_count = (int)(sizeof(s_posts) / sizeof(s_posts[0]));

static Reply s_replies_post1[] = {
    { .id="r1", .author="alice",  .body="Thanks for the welcome!", .created_at=1700000010000ULL },
    { .id="r2", .author="bob",    .body="Happy to be here :)",     .created_at=1700000020000ULL },
};

int mock_get_post_list(int page, PostList* out) {
    (void)page;
    if (!out) return 0;
    out->items = s_posts;
    out->count = s_posts_count;
    out->total = s_posts_count;
    out->page  = 1;
    return 1;
}

int mock_get_post_detail(const char* post_id, Post* out, ReplyList* out_replies) {
    if (!post_id || !out) return 0;
    for (int i = 0; i < s_posts_count; ++i) {
        if (strcmp(s_posts[i].id, post_id) == 0) {
            *out = s_posts[i];
            if (out_replies) {
                if (strcmp(post_id, "post-1") == 0) {
                    out_replies->items = s_replies_post1;
                    out_replies->count = (int)(sizeof(s_replies_post1)/sizeof(s_replies_post1[0]));
                } else {
                    out_replies->items = NULL;
                    out_replies->count = 0;
                }
            }
            return 1;
        }
    }
    return 0;
}

int mock_create_post(const char* title, const char* body) {
    /* Mock: pretend success. */
    (void)title; (void)body;
    return 1;
}

int mock_create_reply(const char* post_id, const char* body) {
    (void)post_id; (void)body;
    return 1;
}

/**
 * Mock chat data (sessions + messages).
 */
#include "../api.h"
#include "../json_parse.h"

#include <stdlib.h>
#include <string.h>

static ChatSession s_sessions[] = {
    { .id="s1", .session_id="s1", .display_name="admin",
      .last_message="Welcome aboard!",     .updated_at=1700000030000ULL, .unread=1 },
    { .id="s2", .session_id="s2", .display_name="retrofan",
      .last_message="Did you try the GBA emu?", .updated_at=1700000040000ULL, .unread=0 },
    { .id="s3", .session_id="s3", .display_name="ftp_dev",
      .last_message="v2 is live",          .updated_at=1700000050000ULL, .unread=2 },
};
static const int s_sessions_count = (int)(sizeof(s_sessions)/sizeof(s_sessions[0]));

static Message s_msgs_s1[] = {
    { .id="m1", .content="Welcome aboard!",   .from_self=0, .timestamp=1700000010000ULL },
    { .id="m2", .content="Thanks, glad to be here.", .from_self=1, .timestamp=1700000020000ULL },
    { .id="m3", .content="Welcome aboard!",   .from_self=0, .timestamp=1700000030000ULL },
};
static Message s_msgs_s2[] = {
    { .id="m4", .content="Did you try the GBA emu?", .from_self=0, .timestamp=1700000040000ULL },
};

int mock_get_chat_sessions(SessionList* out) {
    if (!out) return 0;
    json_free_session_list(out);
    out->items = calloc(s_sessions_count, sizeof(ChatSession));
    if (!out->items) return 0;
    memcpy(out->items, s_sessions, sizeof(s_sessions));
    out->count = s_sessions_count;
    return 1;
}

int mock_get_messages(const char* session_id, MessageList* out) {
    if (!session_id || !out) return 0;
    json_free_message_list(out);
    if (strcmp(session_id, "s1") == 0) {
        int count = (int)(sizeof(s_msgs_s1)/sizeof(s_msgs_s1[0]));
        out->items = calloc(count, sizeof(Message));
        if (out->items) {
            memcpy(out->items, s_msgs_s1, sizeof(s_msgs_s1));
            out->count = count;
        }
    } else if (strcmp(session_id, "s2") == 0) {
        int count = (int)(sizeof(s_msgs_s2)/sizeof(s_msgs_s2[0]));
        out->items = calloc(count, sizeof(Message));
        if (out->items) {
            memcpy(out->items, s_msgs_s2, sizeof(s_msgs_s2));
            out->count = count;
        }
    } else {
        out->items = NULL;
        out->count = 0;
    }
    return 1;
}

int mock_send_message(const char* session_id, const char* content) {
    (void)session_id; (void)content;
    return 1;
}

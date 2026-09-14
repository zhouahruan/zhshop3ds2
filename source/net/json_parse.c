#include "json_parse.h"
#include "../core/utils.h"

#include <jansson.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/*---- Helpers ----*/

static void copy_str(char* dst, size_t cap, const char* src) {
    if (!src) { if (cap > 0) dst[0] = '\0'; return; }
    utils_strlcpy(dst, src, cap);
}

static const char* obj_string(const json_t* o, const char* key) {
    json_t* v = json_object_get(o, key);
    return json_is_string(v) ? json_string_value(v) : NULL;
}

static json_int_t obj_int(const json_t* o, const char* key) {
    json_t* v = json_object_get(o, key);
    return json_is_integer(v) ? json_integer_value(v) : 0;
}

static double obj_real(const json_t* o, const char* key) {
    json_t* v = json_object_get(o, key);
    return json_is_real(v) ? json_real_value(v) :
           json_is_integer(v) ? (double)json_integer_value(v) : 0.0;
}

static uint8_t obj_bool(const json_t* o, const char* key) {
    json_t* v = json_object_get(o, key);
    return json_is_true(v) ? 1 : 0;
}

/* Unwrap the API envelope: {"code":0,"data":<payload>}.
 * If the root has a "data" key, return that; otherwise return root.
 * If "code" is non-zero, treat as error (return NULL). */
static json_t* unwrap_envelope(json_t* root) {
    if (!root) return NULL;
    json_t* code_v = json_object_get(root, "code");
    if (code_v) {
        /* Envelope present. Check code==0 for success. */
        if (json_is_integer(code_v) && json_integer_value(code_v) != 0)
            return NULL;
        if (json_is_real(code_v) && json_real_value(code_v) != 0.0)
            return NULL;
        json_t* data = json_object_get(root, "data");
        if (data) return data;
    }
    return root;
}

/*---- App ----*/

static void parse_app_into(const json_t* o, App* a) {
    memset(a, 0, sizeof(*a));
    copy_str(a->id,            sizeof(a->id),            obj_string(o, "id"));
    copy_str(a->slug,          sizeof(a->slug),          obj_string(o, "slug"));
    copy_str(a->name,          sizeof(a->name),          obj_string(o, "name"));
    copy_str(a->version,       sizeof(a->version),       obj_string(o, "version"));
    copy_str(a->short_desc,    sizeof(a->short_desc),    obj_string(o, "short_desc"));
    copy_str(a->description,    sizeof(a->description),   obj_string(o, "description"));
    copy_str(a->icon_url,      sizeof(a->icon_url),      obj_string(o, "icon_url"));
    const char* dev = obj_string(o, "developer_name");
    json_t* dev_obj = json_object_get(o, "developer");
    if (!dev && json_is_object(dev_obj))
        dev = obj_string(dev_obj, "username");
    copy_str(a->developer_name, sizeof(a->developer_name), dev);
    a->avg_rating      = (float)obj_real(o, "avg_rating");
    a->rating_count    = (int)obj_int(o, "rating_count");
    a->download_count  = (int)obj_int(o, "download_count");
    a->package_size    = (int)obj_int(o, "package_size");
    if (a->package_size <= 0)
        a->package_size = (int)obj_int(o, "size_bytes");

    const char* cid = obj_string(o, "category_id");
    copy_str(a->category_id, sizeof(a->category_id), cid);
    json_t* cat_obj = json_object_get(o, "category");
    if (json_is_object(cat_obj)) {
        if (!cid) cid = obj_string(cat_obj, "id");
        if (cid) copy_str(a->category_id, sizeof(a->category_id), cid);
        copy_str(a->category_name, sizeof(a->category_name),
                 obj_string(cat_obj, "name"));
    }

    copy_str(a->region,          sizeof(a->region),          obj_string(o, "region"));
    copy_str(a->min_firmware,    sizeof(a->min_firmware),    obj_string(o, "min_firmware"));
    a->cfw_required      = obj_bool(o, "cfw_required");
    copy_str(a->title_id,        sizeof(a->title_id),        obj_string(o, "title_id"));
    copy_str(a->file_format,     sizeof(a->file_format),     obj_string(o, "file_format"));
    a->new3ds_exclusive  = obj_bool(o, "new3ds_exclusive");
    copy_str(a->app_source,      sizeof(a->app_source),      obj_string(o, "app_source"));
    copy_str(a->license_type,    sizeof(a->license_type),    obj_string(o, "license_type"));
    copy_str(a->created_at,      sizeof(a->created_at),      obj_string(o, "created_at"));
    copy_str(a->updated_at,      sizeof(a->updated_at),      obj_string(o, "updated_at"));
    copy_str(a->version_updated_at, sizeof(a->version_updated_at), obj_string(o, "version_updated_at"));

    json_t* shots = json_object_get(o, "screenshots");
    if (!shots) shots = json_object_get(o, "screenshot_urls");
    if (json_is_array(shots)) {
        size_t i, n = json_array_size(shots);
        if (n > SCREENSHOT_MAX) n = SCREENSHOT_MAX;
        for (i = 0; i < n; ++i) {
            json_t* s = json_array_get(shots, i);
            const char* url = NULL;
            if (json_is_string(s)) {
                url = json_string_value(s);
            } else if (json_is_object(s)) {
                url = obj_string(s, "url");
                if (!url) url = obj_string(s, "image_url");
            }
            copy_str(a->screenshot_urls[i], URL_LEN, url);
        }
        a->screenshot_count = (int)n;
    }
}

int json_parse_app_list(const char* body, AppList* out) {
    if (!body || !out) return 0;
    memset(out, 0, sizeof(*out));
    json_error_t err;
    json_t* root = json_loads(body, 0, &err);
    if (!root) return 0;

    /* Unwrap envelope {"code":0,"data":{...}}. */
    json_t* payload = unwrap_envelope(root);
    if (!payload) { json_decref(root); return 0; }

    /* payload may be the inner object with "list", or an array. */
    json_t* items = json_object_get(payload, "list");
    if (!json_is_array(items)) items = json_object_get(payload, "items");
    if (!json_is_array(items)) {
        /* Tolerate: payload itself might be an array. */
        if (json_is_array(payload)) items = payload;
    }
    if (!json_is_array(items)) { json_decref(root); return 0; }
    size_t n = json_array_size(items);
    out->items = calloc(n, sizeof(App));
    if (!out->items) { json_decref(root); return 0; }
    for (size_t i = 0; i < n; ++i) {
        parse_app_into(json_array_get(items, i), &out->items[i]);
    }
    out->count     = (int)n;
    out->total     = (int)obj_int(payload, "total");
    out->page      = (int)obj_int(payload, "page");
    out->page_size = (int)obj_int(payload, "page_size");
    json_decref(root);
    return 1;
}

void json_free_app_list(AppList* p) {
    if (!p) return;
    if (p->items) free(p->items);
    p->items = NULL; p->count = p->total = p->page = 0;
}

int json_parse_category_list(const char* body, CategoryList* out) {
    if (!body || !out) return 0;
    memset(out, 0, sizeof(*out));
    json_error_t err;
    json_t* root = json_loads(body, 0, &err);
    if (!root) return 0;

    json_t* payload = unwrap_envelope(root);
    if (!payload) { json_decref(root); return 0; }

    /* Categories: data is an array directly, or has "items"/"list". */
    json_t* items = json_is_array(payload) ? payload : json_object_get(payload, "items");
    if (!json_is_array(items)) items = json_object_get(payload, "list");
    if (!json_is_array(items)) { json_decref(root); return 0; }
    size_t n = json_array_size(items);
    out->items = calloc(n, sizeof(Category));
    if (!out->items) { json_decref(root); return 0; }
    for (size_t i = 0; i < n; ++i) {
        json_t* o = json_array_get(items, i);
        copy_str(out->items[i].id,       sizeof(out->items[i].id),       obj_string(o, "id"));
        copy_str(out->items[i].zone_id,  sizeof(out->items[i].zone_id),  obj_string(o, "zone_id"));
        copy_str(out->items[i].name,     sizeof(out->items[i].name),     obj_string(o, "name"));
        out->items[i].sort_order = (int)obj_int(o, "sort_order");
    }
    out->count = (int)n;
    json_decref(root);
    return 1;
}

void json_free_category_list(CategoryList* p) {
    if (!p) return;
    if (p->items) free(p->items);
    p->items = NULL; p->count = 0;
}

int json_parse_app_detail(const char* body, AppDetail* out) {
    if (!body || !out) return 0;
    memset(out, 0, sizeof(*out));
    json_error_t err;
    json_t* root = json_loads(body, 0, &err);
    if (!root) return 0;

    json_t* payload = unwrap_envelope(root);
    if (!payload) { json_decref(root); return 0; }

    parse_app_into(payload, &out->base);

    const char* desc = obj_string(payload, "description");
    if (desc) copy_str(out->long_description, sizeof(out->long_description), desc);
    else copy_str(out->long_description, sizeof(out->long_description), out->base.description);
    copy_str(out->changelog, sizeof(out->changelog), obj_string(payload, "changelog"));

    copy_str(out->package_url,       sizeof(out->package_url),       obj_string(payload, "package_url"));
    copy_str(out->developer_website, sizeof(out->developer_website), obj_string(payload, "developer_website"));
    copy_str(out->developer_email,   sizeof(out->developer_email),   obj_string(payload, "developer_email"));
    out->has_ads       = obj_bool(payload, "has_ads");
    out->requires_root = obj_bool(payload, "requires_root");
    copy_str(out->permissions, sizeof(out->permissions), obj_string(payload, "permissions"));

    json_t* zone = json_object_get(payload, "zone");
    if (json_is_object(zone))
        copy_str(out->zone_name, sizeof(out->zone_name), obj_string(zone, "name"));

    json_t* up = json_object_get(payload, "uploader");
    if (json_is_object(up)) {
        copy_str(out->uploader_username, sizeof(out->uploader_username), obj_string(up, "username"));
        copy_str(out->uploader_avatar,   sizeof(out->uploader_avatar),   obj_string(up, "avatar_url"));
        out->uploader_certified = obj_bool(up, "is_certified");
    }
    json_decref(root);
    return 1;
}

int json_parse_download_info(const char* body, char* out_url, int url_cap,
                             int* out_size) {
    (void)body;
    if (!out_url) return 0;
    out_url[0] = '\0';
    if (out_size) *out_size = 0;
    return 1;
}

/*---- Forum ----*/

static void parse_post_into(const json_t* o, Post* p) {
    copy_str(p->id,     sizeof(p->id),     obj_string(o, "id"));
    copy_str(p->title,  sizeof(p->title),  obj_string(o, "title"));
    copy_str(p->author, sizeof(p->author), obj_string(o, "author"));
    copy_str(p->body,   sizeof(p->body),   obj_string(o, "body"));
    p->reply_count = (int)obj_int(o, "reply_count");
    p->created_at  = (u64)obj_int(o, "created_at");
}

int json_parse_post_list(const char* body, PostList* out) {
    if (!body || !out) return 0;
    memset(out, 0, sizeof(*out));
    json_error_t err;
    json_t* root = json_loads(body, 0, &err);
    if (!root) return 0;

    json_t* payload = unwrap_envelope(root);
    if (!payload) { json_decref(root); return 0; }

    json_t* items = json_object_get(payload, "items");
    if (!json_is_array(items)) items = json_object_get(payload, "list");
    if (!json_is_array(items)) { json_decref(root); return 0; }
    size_t n = json_array_size(items);
    out->items = calloc(n, sizeof(Post));
    if (!out->items) { json_decref(root); return 0; }
    for (size_t i = 0; i < n; ++i) parse_post_into(json_array_get(items, i), &out->items[i]);
    out->count = (int)n;
    out->total = (int)obj_int(payload, "total");
    out->page  = (int)obj_int(payload, "page");
    json_decref(root);
    return 1;
}

void json_free_post_list(PostList* p) {
    if (!p) return;
    if (p->items) free(p->items);
    p->items = NULL; p->count = p->total = p->page = 0;
}

int json_parse_post_detail(const char* body, Post* out, ReplyList* out_replies) {
    if (!body || !out) return 0;
    json_error_t err;
    json_t* root = json_loads(body, 0, &err);
    if (!root) return 0;

    json_t* payload = unwrap_envelope(root);
    if (!payload) { json_decref(root); return 0; }

    parse_post_into(payload, out);
    if (out_replies) {
        json_t* items = json_object_get(payload, "replies");
        if (json_is_array(items)) {
            size_t n = json_array_size(items);
            out_replies->items = calloc(n, sizeof(Reply));
            if (out_replies->items) {
                for (size_t i = 0; i < n; ++i) {
                    json_t* o = json_array_get(items, i);
                    copy_str(out_replies->items[i].id,     sizeof(out_replies->items[i].id),     obj_string(o, "id"));
                    copy_str(out_replies->items[i].author, sizeof(out_replies->items[i].author), obj_string(o, "author"));
                    copy_str(out_replies->items[i].body,   sizeof(out_replies->items[i].body),   obj_string(o, "body"));
                    out_replies->items[i].created_at = (u64)obj_int(o, "created_at");
                }
                out_replies->count = (int)n;
            }
        }
    }
    json_decref(root);
    return 1;
}

void json_free_reply_list(ReplyList* p) {
    if (!p) return;
    if (p->items) free(p->items);
    p->items = NULL; p->count = 0;
}

/*---- Chat ----*/

int json_parse_session_list(const char* body, SessionList* out) {
    if (!body || !out) return 0;
    memset(out, 0, sizeof(*out));
    json_error_t err;
    json_t* root = json_loads(body, 0, &err);
    if (!root) return 0;

    json_t* payload = unwrap_envelope(root);
    if (!payload) { json_decref(root); return 0; }

    json_t* items = json_object_get(payload, "items");
    if (!json_is_array(items)) items = json_object_get(payload, "list");
    if (!json_is_array(items)) { json_decref(root); return 0; }
    size_t n = json_array_size(items);
    out->items = calloc(n, sizeof(ChatSession));
    if (!out->items) { json_decref(root); return 0; }
    for (size_t i = 0; i < n; ++i) {
        json_t* o = json_array_get(items, i);
        copy_str(out->items[i].id,            sizeof(out->items[i].id),            obj_string(o, "id"));
        copy_str(out->items[i].session_id,   sizeof(out->items[i].session_id),   obj_string(o, "session_id"));
        copy_str(out->items[i].display_name,  sizeof(out->items[i].display_name),  obj_string(o, "display_name"));
        copy_str(out->items[i].last_message,  sizeof(out->items[i].last_message),  obj_string(o, "last_message"));
        out->items[i].updated_at = (u64)obj_int(o, "updated_at");
        out->items[i].unread     = (int)obj_int(o, "unread");
    }
    out->count = (int)n;
    json_decref(root);
    return 1;
}

void json_free_session_list(SessionList* p) {
    if (!p) return;
    if (p->items) free(p->items);
    p->items = NULL; p->count = 0;
}

int json_parse_message_list(const char* body, MessageList* out) {
    if (!body || !out) return 0;
    memset(out, 0, sizeof(*out));
    json_error_t err;
    json_t* root = json_loads(body, 0, &err);
    if (!root) return 0;

    json_t* payload = unwrap_envelope(root);
    if (!payload) { json_decref(root); return 0; }

    json_t* items = json_object_get(payload, "items");
    if (!json_is_array(items)) items = json_object_get(payload, "list");
    if (!json_is_array(items)) { json_decref(root); return 0; }
    size_t n = json_array_size(items);
    out->items = calloc(n, sizeof(Message));
    if (!out->items) { json_decref(root); return 0; }
    for (size_t i = 0; i < n; ++i) {
        json_t* o = json_array_get(items, i);
        copy_str(out->items[i].id,       sizeof(out->items[i].id),       obj_string(o, "id"));
        copy_str(out->items[i].content, sizeof(out->items[i].content), obj_string(o, "content"));
        out->items[i].from_self = (int)obj_int(o, "from_self");
        out->items[i].timestamp = (u64)obj_int(o, "timestamp");
    }
    out->count = (int)n;
    json_decref(root);
    return 1;
}

void json_free_message_list(MessageList* p) {
    if (!p) return;
    if (p->items) free(p->items);
    p->items = NULL; p->count = 0;
}

int json_parse_user(const char* body, User* out) {
    if (!body || !out) return 0;
    memset(out, 0, sizeof(*out));
    json_error_t err;
    json_t* root = json_loads(body, 0, &err);
    if (!root) return 0;

    json_t* payload = unwrap_envelope(root);
    if (!payload) { json_decref(root); return 0; }

    out->user_id = (int)obj_int(payload, "user_id");
    copy_str(out->username, sizeof(out->username), obj_string(payload, "username"));
    copy_str(out->token,     sizeof(out->token),     obj_string(payload, "token"));
    out->is_login = 1;
    json_decref(root);
    return 1;
}

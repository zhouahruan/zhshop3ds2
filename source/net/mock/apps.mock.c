/**
 * Mock app/category/recommend data. Returned by api.c when USE_MOCK=1.
 * Static — no heap allocation; safe to call repeatedly.
 */
#include "../api.h"
#include "../json_parse.h"
#include "../../core/utils.h"

#include <stdlib.h>
#include <string.h>

static App s_apps[] = {
    {
        .id="app-1", .name="Tetris 3DS", .developer_name="Homebrew Co.",
        .description="Classic block stacking game, 3DS native port.",
        .icon_url="romfs:/icons/tetris.png",
        .screenshot_count=2,
        .screenshot_urls={ "romfs:/shots/tetris1.png", "romfs:/shots/tetris2.png" },
        .avg_rating=4.5f, .download_count=12300, .package_size=2*1024*1024,
        .category_id="games", .category_name="Games",
        .version="1.2", .region="USA", .file_format=".cia",
        .cfw_required=1, .title_id="0004000000164800"
    },
    {
        .id="app-2", .name="PaintChats", .developer_name="Chat Labs",
        .description="Draw with friends over Wi-Fi. Send strokes in real time.",
        .icon_url="romfs:/icons/paintchat.png",
        .screenshot_count=1,
        .screenshot_urls={ "romfs:/shots/paintchat.png" },
        .avg_rating=4.0f, .download_count=4520, .package_size=3*1024*1024,
        .category_id="social", .category_name="Social",
        .version="0.9", .region="Region Free", .file_format=".cia",
        .cfw_required=1
    },
    {
        .id="app-3", .name="FTP Client", .developer_name="devkitPro",
        .description="Browse and transfer files over FTP from your 3DS.",
        .icon_url="romfs:/icons/ftp.png",
        .screenshot_count=2,
        .screenshot_urls={ "romfs:/shots/ftp1.png", "romfs:/shots/ftp2.png" },
        .avg_rating=4.8f, .download_count=8900, .package_size=512*1024,
        .category_id="tools", .category_name="Tools",
        .version="2.0", .region="Region Free", .file_format=".cia",
        .cfw_required=1
    },
    {
        .id="app-4", .name="GBA Emulator", .developer_name="homebrew",
        .description="Run Game Boy Advance ROMs on the 3DS.",
        .icon_url="romfs:/icons/gba.png",
        .screenshot_count=3,
        .screenshot_urls={ "romfs:/shots/gba1.png","romfs:/shots/gba2.png","romfs:/shots/gba3.png" },
        .avg_rating=4.7f, .download_count=23100, .package_size=5*1024*1024,
        .category_id="emulators", .category_name="Emulators",
        .version="1.4", .region="Region Free", .file_format=".cia",
        .cfw_required=1
    },
    {
        .id="app-5", .name="Weather 3DS", .developer_name="Sky Apps",
        .description="Check local weather from the top screen.",
        .icon_url="romfs:/icons/weather.png",
        .screenshot_count=1,
        .screenshot_urls={ "romfs:/shots/weather.png" },
        .avg_rating=3.5f, .download_count=1200, .package_size=768*1024,
        .category_id="lifestyle", .category_name="Lifestyle",
        .version="1.0", .region="USA", .file_format=".cia",
        .cfw_required=0
    },
    {
        .id="app-6", .name="Music Player", .developer_name="Audio Labs",
        .description="Play MP3, BCSTM and other 3DS audio formats.",
        .icon_url="romfs:/icons/music.png",
        .screenshot_count=2,
        .screenshot_urls={ "romfs:/shots/music1.png","romfs:/shots/music2.png" },
        .avg_rating=4.2f, .download_count=8700, .package_size=4*1024*1024,
        .category_id="media", .category_name="Media",
        .version="3.1", .region="Region Free", .file_format=".cia",
        .cfw_required=1
    },
};
static const int s_apps_count = (int)(sizeof(s_apps) / sizeof(s_apps[0]));

static Category s_categories[] = {
    { .id="all",         .zone_id="3ds", .name="All",         .sort_order=1 },
    { .id="games",       .zone_id="3ds", .name="Games",       .sort_order=2 },
    { .id="tools",       .zone_id="3ds", .name="Tools",       .sort_order=3 },
    { .id="emulators",   .zone_id="3ds", .name="Emulators",   .sort_order=4 },
    { .id="media",       .zone_id="3ds", .name="Media",       .sort_order=5 },
    { .id="social",      .zone_id="3ds", .name="Social",      .sort_order=6 },
    { .id="lifestyle",   .zone_id="3ds", .name="Lifestyle",   .sort_order=7 },
};
static const int s_categories_count = (int)(sizeof(s_categories) / sizeof(s_categories[0]));

int mock_get_recommend(AppList* out) {
    if (!out) return 0;
    json_free_app_list(out);
    out->items = calloc(s_apps_count, sizeof(App));
    if (!out->items) return 0;
    memcpy(out->items, s_apps, sizeof(s_apps));
    out->count = s_apps_count;
    out->total = s_apps_count;
    out->page  = 1;
    return 1;
}

int mock_get_categories(CategoryList* out) {
    if (!out) return 0;
    json_free_category_list(out);
    out->items = calloc(s_categories_count, sizeof(Category));
    if (!out->items) return 0;
    memcpy(out->items, s_categories, sizeof(s_categories));
    out->count = s_categories_count;
    return 1;
}

int mock_get_app_list(const char* category_id, const char* keyword,
                      const char* sort,
                      const char* region, int cfw_required,
                      const char* min_firmware,
                      int page, int page_size, AppList* out) {
    (void)sort; (void)page; (void)page_size;
    (void)region; (void)cfw_required; (void)min_firmware;
    if (!out) return 0;
    json_free_app_list(out);
    out->items = calloc(s_apps_count, sizeof(App));
    if (!out->items) return 0;
    memcpy(out->items, s_apps, sizeof(s_apps));
    out->count = s_apps_count;
    out->total = s_apps_count;
    out->page  = 1;
    (void)category_id; (void)keyword;
    return 1;
}

int mock_get_app_detail(const char* app_id, AppDetail* out) {
    if (!app_id || !out) return 0;
    memset(out, 0, sizeof(*out));
    for (int i = 0; i < s_apps_count; ++i) {
        if (strcmp(s_apps[i].id, app_id) == 0) {
            out->base = s_apps[i];
            utils_strlcpy(out->long_description, s_apps[i].description,
                          sizeof(out->long_description));
            utils_strlcpy(out->changelog, "First stable release.",
                          sizeof(out->changelog));
            return 1;
        }
    }
    return 0;
}

int mock_get_download_url(const char* app_id, char* out_url, int url_cap,
                          int* out_size) {
    if (!app_id || !out_url) return 0;
    for (int i = 0; i < s_apps_count; ++i) {
        if (strcmp(s_apps[i].id, app_id) == 0) {
            /* In real mode this would be a real https url.
             * Mock: pretend file is at romfs path. */
            utils_strlcpy(out_url, "romfs:/cias/app.cia", (size_t)url_cap);
            if (out_size) *out_size = s_apps[i].package_size;
            return 1;
        }
    }
    return 0;
}

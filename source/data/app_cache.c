#include "app_cache.h"
#include "../core/utils.h"

#include <3ds.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

static void ensure_dir(void) {
    /* Best-effort: create the path via mkdir() on the sdmc: mount. */
    mkdir("sdmc:/3ds",            0777);
    mkdir("sdmc:/3ds/appstore",   0777);
    mkdir(CACHE_DIR,              0777);
}

int cache_save_app_list(const char* key, const char* body) {
    if (!key || !body) return 0;
    ensure_dir();
    char path[256];
    snprintf(path, sizeof(path), "%s/%s.json", CACHE_DIR, key);
    FILE* fp = fopen(path, "wb");
    if (!fp) return 0;
    size_t n = fwrite(body, 1, strlen(body), fp);
    fclose(fp);
    return n > 0 ? 1 : 0;
}

int cache_load_app_list(const char* key, char* buf, size_t cap) {
    if (!key || !buf || cap == 0) return 0;
    char path[256];
    snprintf(path, sizeof(path), "%s/%s.json", CACHE_DIR, key);
    FILE* fp = fopen(path, "rb");
    if (!fp) return 0;
    size_t n = fread(buf, 1, cap - 1, fp);
    fclose(fp);
    buf[n] = '\0';
    return n > 0 ? 1 : 0;
}

void cache_clear(void) {
    /* Best-effort: remove known keys. */
    remove(CACHE_DIR "/recommend.json");
    remove(CACHE_DIR "/categories.json");
    remove(CACHE_DIR "/applist.json");
}

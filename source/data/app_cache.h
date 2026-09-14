/**
 * Simple persistent cache backed by SD file system.
 *
 * Saves the most recent AppList as raw JSON text in sdmc:/3ds/appstore/cache/
 * so the home screen can render something even if the network is down
 * on launch.
 */
#ifndef DATA_APP_CACHE_H
#define DATA_APP_CACHE_H

#include "types.h"

#define CACHE_DIR "sdmc:/3ds/appstore/cache"

int  cache_save_app_list(const char* key, const char* body);
int  cache_load_app_list(const char* key, char* buf, size_t cap);
void cache_clear(void);

#endif /* DATA_APP_CACHE_H */

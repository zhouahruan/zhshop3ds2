#include "downloader.h"
#include "../net/http.h"
#include "../data/store.h"
#include "../core/utils.h"
#include "installer.h"

#include <3ds.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>

/* Each active download runs in its own worker thread because httpc is
 * blocking. The main thread polls progress via downloader_tick(). */
typedef struct {
    Thread    thread;
    char      save_path[256];
    char      url[512];
    int       store_idx;
    uint8_t   active;
    uint8_t   done;
    int       result;  /* 0 = ok, <0 = fail */
} TaskHandle;

#define MAX_TASKS 16
#define STACK_SIZE (32 * 1024)

static TaskHandle s_handles[MAX_TASKS];

static TaskHandle* alloc_handle(int store_idx) {
    for (int i = 0; i < MAX_TASKS; ++i) {
        if (!s_handles[i].active) {
            memset(&s_handles[i], 0, sizeof(s_handles[i]));
            s_handles[i].store_idx = store_idx;
            s_handles[i].active = 1;
            return &s_handles[i];
        }
    }
    return NULL;
}

static void free_handle(TaskHandle* h) {
    if (!h) return;
    if (h->thread) {
        threadJoin(h->thread, U64_MAX);
        threadFree(h->thread);
        h->thread = NULL;
    }
    h->active = 0;
}

static void download_thread(void* arg) {
    TaskHandle* h = (TaskHandle*)arg;
    int idx = h->store_idx;
    DownloadTask* t = (idx >= 0 && idx < g_store.download_count)
                     ? &g_store.downloads[idx] : NULL;

    /* Make sure the target directory exists. */
    mkdir("sdmc:/3ds", 0777);
    mkdir("sdmc:/3ds/appstore", 0777);
    mkdir("sdmc:/3ds/appstore/cias", 0777);

    NetStatus st = http_download(h->url, h->save_path, NULL, NULL);
    h->result = (st == NET_OK) ? 0 : -1;

    if (t) {
        if (st == NET_OK) {
            t->status = DL_DONE;
            t->progress = 100;
            t->downloaded_bytes = t->total_bytes;
            /* Auto-install the downloaded CIA. */
            installer_install_cia(h->save_path);
        } else {
            t->status = DL_FAILED;
        }
    }
    h->done = 1;
}

void downloader_init(void) {
    memset(s_handles, 0, sizeof(s_handles));
}

void downloader_exit(void) {
    for (int i = 0; i < MAX_TASKS; ++i) {
        if (s_handles[i].active) free_handle(&s_handles[i]);
    }
}

int downloader_start(int idx) {
    if (idx < 0 || idx >= g_store.download_count) return -1;
    DownloadTask* t = &g_store.downloads[idx];
    if (t->status == DL_RUNNING) return 0;
    if (t->download_url[0] == '\0') return -1;

    /* If a handle already exists for this index, just restart. */
    for (int i = 0; i < MAX_TASKS; ++i) {
        if (s_handles[i].active && s_handles[i].store_idx == idx) {
            free_handle(&s_handles[i]);
            break;
        }
    }

    TaskHandle* h = alloc_handle(idx);
    if (!h) return -1;

    utils_strlcpy(h->url, t->download_url, sizeof(h->url));
    snprintf(h->save_path, sizeof(h->save_path),
             "sdmc:/3ds/appstore/cias/%s.cia", t->app_id);

    t->status     = DL_RUNNING;
    t->started_at = osGetTime();

    h->thread = threadCreate(download_thread, h, STACK_SIZE, 0x18, -2, true);
    if (!h->thread) {
        h->active = 0;
        t->status = DL_FAILED;
        return -1;
    }
    return 0;
}

void downloader_pause(int idx) {
    /* httpc has no clean pause; mark paused and let the thread finish
     * reading (data will be discarded on resume by overwriting). */
    if (idx >= 0 && idx < g_store.download_count) {
        g_store.downloads[idx].status = DL_PAUSED;
    }
}

void downloader_resume(int idx) {
    downloader_start(idx);
}

void downloader_cancel(int idx) {
    for (int i = 0; i < MAX_TASKS; ++i) {
        if (s_handles[i].active && s_handles[i].store_idx == idx) {
            free_handle(&s_handles[i]);
            for (int j = 0; j < MAX_TASKS; ++j) {
                if (s_handles[j].active && s_handles[j].store_idx > idx) {
                    s_handles[j].store_idx--;
                }
            }
            break;
        }
    }
    store_remove_download(idx);
}

void downloader_tick(void) {
    for (int i = 0; i < MAX_TASKS; ++i) {
        if (!s_handles[i].active) continue;
        TaskHandle* h = &s_handles[i];
        int idx = h->store_idx;
        if (idx < 0 || idx >= g_store.download_count) continue;
        DownloadTask* t = &g_store.downloads[idx];

        if (h->done) {
            free_handle(h);
            continue;
        }

        if (t->status != DL_RUNNING) continue;

        /* Update progress from the file on disk (best-effort). */
        FILE* fp = fopen(h->save_path, "rb");
        if (fp) {
            fseek(fp, 0, SEEK_END);
            long pos = ftell(fp);
            fclose(fp);
            if (pos >= 0) {
                t->downloaded_bytes = (int)pos;
                if (t->total_bytes > 0) {
                    t->progress = (int)(pos * 100 / t->total_bytes);
                    if (t->progress > 99) t->progress = 99;
                }
            }
        }
    }
}

int downloader_has_active(void) {
    for (int i = 0; i < MAX_TASKS; ++i) {
        if (s_handles[i].active && !s_handles[i].done) return 1;
    }
    return 0;
}

/**
 * Async download task management via libcurl multi.
 *
 * One background CURLM handle drives all active tasks; main loop calls
 * downloader_tick() each frame which calls curl_multi_perform() and
 * updates store.downloads[i].progress / status. When a download
 * completes the cia file path is ready for installer_install_cia().
 *
 * Public API is intentionally small — start/pause/cancel/clear; the
 * per-task status fields live in store.downloads[].
 */
#ifndef DOWNLOAD_DOWNLOADER_H
#define DOWNLOAD_DOWNLOADER_H

#include <3ds.h>

void downloader_init(void);
void downloader_exit(void);

/* Kick off (or resume) the task at index idx in g_store.downloads.
 * Returns 0 on success, <0 on failure. */
int  downloader_start(int idx);

/* Pause / resume a running task. */
void downloader_pause(int idx);
void downloader_resume(int idx);

/* Cancel and remove the task from the active set. */
void downloader_cancel(int idx);

/* Per-frame tick — main loop calls this once per frame. */
void downloader_tick(void);

/* True if any task is currently in DL_RUNNING state. */
int  downloader_has_active(void);

#endif /* DOWNLOAD_DOWNLOADER_H */

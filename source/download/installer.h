/**
 * CIA install via the AM service.
 *
 * Used by downloader.c when a download finishes. Also exposed to
 * scenes that need to install a user-supplied cia file (long-press
 * in the file browser, etc.).
 *
 * Returns 0 on success, negative on failure.
 */
#ifndef DOWNLOAD_INSTALLER_H
#define DOWNLOAD_INSTALLER_H

#include <3ds.h>

int installer_init(void);
void installer_exit(void);

/* Install a CIA from SD path. After install the title appears in the
 * HOME Menu. Returns 0 on success. */
int installer_install_cia(const char* cia_path);

/* Uninstall title by title id. */
int installer_uninstall(u64 title_id);

/* Returns a 0..100 progress value during an in-progress install. */
int installer_progress(void);

#endif /* DOWNLOAD_INSTALLER_H */

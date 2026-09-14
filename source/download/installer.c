#include "installer.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static Handle s_am_handle = 0;
static int    s_progress = 0;

int installer_init(void) {
    Result rc = amInit();
    if (R_FAILED(rc)) return -1;
    /* Get the default AM handle. */
    rc = AM_GetTitleCount(MEDIATYPE_SD, (u32*)&s_am_handle);
    /* Some versions require AM_GetTitleInfo; we keep handle 0 and
     * just rely on amInit for AM_InstallCia which uses the global
     * session. */
    return 0;
}

void installer_exit(void) {
    amExit();
}

static Result copy_to_temp(const char* cia_path, u64* out_size) {
    FILE* fp = fopen(cia_path, "rb");
    if (!fp) return -1;

    fseek(fp, 0, SEEK_END);
    long size = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    if (size <= 0) { fclose(fp); return -1; }

    /* AM_InstallCia needs a buffered file handle. Use bufferedRead. */
    Handle fh;
    FS_Path archPath, filePath;
    /* On real 3DS the path string must be UTF-16 in the form u:/<path>.
     * We translate sdmc:/ -> u:/. */
    char upath[256];
    snprintf(upath, sizeof(upath), "u:/%s", cia_path + 5); /* skip "sdmc" */
    ssize_t units = utf8_to_utf16((u16*)upath, (const u8*)upath, sizeof(upath));
    if (units < 0) { fclose(fp); return -1; }

    /* Use FSUSER to open a buffered cia read handle. */
    archPath = (FS_Path){ PATH_EMPTY, 0, NULL };
    filePath = (FS_Path){ PATH_UTF16, units * 2, upath };

    Result rc = FSUSER_OpenFileDirectly(&fh, ARCHIVE_SDMC, archPath,
                                         filePath, FS_OPEN_READ,
                                         FS_OPEN_READ);
    if (R_FAILED(rc)) { fclose(fp); return rc; }

    Handle ciaHandle;
    rc = AM_StartCiaInstall(MEDIATYPE_SD, &ciaHandle);
    if (R_FAILED(rc)) { fclose(fp); FSFILE_Close(fh); return rc; }

    /* Stream the file. */
    u8* buf = (u8*)malloc(0x10000);
    if (!buf) { FSFILE_Close(fh); fclose(fp); AM_CancelCIAInstall(ciaHandle); return -1; }

    u64 bytes_read = 0;
    u32 total = (u32)size;
    s_progress = 0;
    while (bytes_read < total) {
        u32 to_read = (total - bytes_read) > 0x10000 ? 0x10000 : (u32)(total - bytes_read);
        u32 got = 0;
        u64 off = bytes_read;
        Result r = FSFILE_Read(fh, &got, off, buf, to_read);
        if (R_FAILED(r)) { free(buf); FSFILE_Close(fh); fclose(fp);
                            AM_CancelCIAInstall(ciaHandle); return r; }
        u32 written = 0;
        r = FSFILE_Write(ciaHandle, &written, bytes_read, buf, got, 0);
        if (R_FAILED(r)) { free(buf); FSFILE_Close(fh); fclose(fp);
                            AM_CancelCIAInstall(ciaHandle); return r; }
        bytes_read += got;
        s_progress = (int)(bytes_read * 100 / total);
    }
    free(buf);
    FSFILE_Close(fh);
    fclose(fp);

    rc = AM_FinishCiaInstall(ciaHandle);
    if (out_size) *out_size = (u64)total;
    s_progress = 100;
    return rc;
}

int installer_install_cia(const char* cia_path) {
    if (!cia_path) return -1;
    s_progress = 0;
    Result rc = copy_to_temp(cia_path, NULL);
    if (R_FAILED(rc)) {
        s_progress = 0;
        return -1;
    }
    return 0;
}

int installer_uninstall(u64 title_id) {
    Result rc = AM_DeleteTitle(MEDIATYPE_SD, title_id);
    return R_SUCCEEDED(rc) ? 0 : -1;
}

int installer_progress(void) {
    return s_progress;
}

#include "utils.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

void utils_strlcpy(char* dst, const char* src, size_t n) {
    if (!dst || n == 0) return;
    if (!src) { dst[0] = '\0'; return; }
    size_t i = 0;
    while (i + 1 < n && src[i]) { dst[i] = src[i]; i++; }
    dst[i] = '\0';
}

void utils_strlcat(char* dst, const char* src, size_t n) {
    if (!dst || !src || n == 0) return;
    size_t dlen = strlen(dst);
    if (dlen >= n - 1) return;
    size_t i = 0;
    while (dlen + i + 1 < n && src[i]) { dst[dlen + i] = src[i]; i++; }
    dst[dlen + i] = '\0';
}

void utils_format_bytes(char* buf, size_t n, uint64_t bytes) {
    if (!buf || n == 0) return;
    const char* units[] = {"B", "KB", "MB", "GB", "TB"};
    int u = 0;
    double v = (double)bytes;
    while (v >= 1024.0 && u < 4) { v /= 1024.0; u++; }
    snprintf(buf, n, "%.1f %s", v, units[u]);
}

void utils_format_time(char* buf, size_t n, uint64_t unix_ms) {
    if (!buf || n == 0) return;
    time_t t = (time_t)(unix_ms / 1000ULL);
    struct tm* tm = gmtime(&t);
    if (tm) {
        snprintf(buf, n, "%04d-%02d-%02d %02d:%02d",
                 tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday,
                 tm->tm_hour, tm->tm_min);
    } else if (n > 0) {
        buf[0] = '\0';
    }
}

int utils_hex_to_int(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return -1;
}

void* utils_alloc(size_t n) {
    void* p = malloc(n);
    return p;
}

void* utils_realloc(void* p, size_t n) {
    return realloc(p, n);
}

void utils_free(void* p) {
    free(p);
}

void utils_free_counted(void** p, int* count) {
    if (!p || !*p) return;
    free(*p);
    *p = NULL;
    if (count) *count = 0;
}

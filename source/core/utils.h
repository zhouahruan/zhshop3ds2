/**
 * General-purpose helpers used across the app: safe string copy,
 * byte/time formatting, simple color helpers.
 */
#ifndef CORE_UTILS_H
#define CORE_UTILS_H

#include <stddef.h>
#include <stdint.h>
#include <3ds.h>

/*---- Safe string copy. dst is always NUL-terminated. ----*/
void utils_strlcpy(char* dst, const char* src, size_t n);
void utils_strlcat(char* dst, const char* src, size_t n);

/* Format byte count to human readable ("4.2 MB"). buf must be >= 16 bytes. */
void utils_format_bytes(char* buf, size_t n, uint64_t bytes);

/* Format a unix timestamp to "YYYY-MM-DD HH:MM". */
void utils_format_time(char* buf, size_t n, uint64_t unix_ms);

/* Convert hex digit to int (0-15) or -1 on failure. */
int  utils_hex_to_int(char c);

/*---- Color helpers (RGBA8 little-endian as expected by citro2d). ----*/
static inline uint32_t rgba8(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    return ((a & 0xFF) << 24) | ((b & 0xFF) << 16) |
           ((g & 0xFF) << 8)  | (r & 0xFF);
}

/*---- Safe alloc. ----*/
void* utils_alloc(size_t n);
void* utils_realloc(void* p, size_t n);
void  utils_free(void* p);
void  utils_free_counted(void** p, int* count);

/*---- Color constants matching eShop palette. ----*/
#define C_BG          rgba8(0x1A, 0x1A, 0x2E, 0xFF)
#define C_SURFACE     rgba8(0xF2, 0xF2, 0xF7, 0xFF)
#define C_TEXT         rgba8(0x16, 0x21, 0x3E, 0xFF)
#define C_MUTED        rgba8(0x6C, 0x75, 0x7D, 0xFF)
#define C_RED          rgba8(0xE6, 0x39, 0x46, 0xFF)
#define C_BLUE         rgba8(0x1D, 0x6F, 0xB8, 0xFF)
#define C_YELLOW       rgba8(0xFF, 0xD2, 0x3F, 0xFF)
#define C_GREEN        rgba8(0x06, 0xA7, 0x7D, 0xFF)
#define C_BEZEL         rgba8(0x0F, 0x0F, 0x1A, 0xFF)
#define C_SCREEN_BG     rgba8(0xED, 0xED, 0xF4, 0xFF)

#endif /* CORE_UTILS_H */

#ifndef COMMON_H
#define COMMON_H

// ===== Standard library =====
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdint.h>
#include <stdbool.h>

// ===== POSIX / filesystem =====
#include <dirent.h>
#include <sys/stat.h>

// ===== Convenience macros =====
#define UNUSED(x) (void)(x)

// Fatal error: print message to stderr and exit non‑zero
#define DIE(...)                               \
    do {                                       \
        fprintf(stderr, __VA_ARGS__);          \
        fputc('\n', stderr);                   \
        exit(EXIT_FAILURE);                    \
    } while (0)

// ===== Safe allocation wrappers =====

static inline void* xmalloc(size_t n) {
    void* p = malloc(n);
    if (!p && n) DIE("Out of memory (malloc %zu bytes)", n);
    return p;
}

static inline void* xcalloc(size_t count, size_t size) {
    void* p = calloc(count, size);
    if (!p && count && size) DIE("Out of memory (calloc %zu x %zu)", count, size);
    return p;
}

static inline void* xrealloc(void* old, size_t n) {
    void* p = realloc(old, n);
    if (!p && n) DIE("Out of memory (realloc %zu bytes)", n);
    return p;
}

  static inline char* xstrdup(const char* s) {
    if (!s) return NULL;
    size_t n = strlen(s) + 1;
    char* p = (char*)xmalloc(n);
    memcpy(p, s, n);
    return p;
}

// ===== Lightweight string helpers =====

static inline char* str_trim_ascii(char* s) {
    if (!s) return s;
    char* p = s;
    while (*p && (*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n' || *p == '\f' || *p == '\v')) ++p;
    if (p != s) memmove(s, p, strlen(p) + 1);
    size_t len = strlen(s);
    while (len && (unsigned char)s[len - 1] <= ' ' ) { 
        s[--len] = '\0';
    }
    return s;
}

// Lowercase a string in-place (ASCII)
static inline char* str_to_lower_ascii(char* s) {
    if (!s) return s;
    for (char* p = s; *p; ++p) *p = (char)tolower((unsigned char)*p);
    return s;
}

// Check if all chars are alphabetic
static inline bool str_is_alpha_ascii(const char* s) {
    if (!s || !*s) return false;
    for (const unsigned char* p = (const unsigned char*)s; *p; ++p) {
        if (!isalpha(*p)) return false;
    }
    return true;
}

// ===== Hash function =====
static inline uint64_t strhash64(const char* s) {
    const uint64_t FNV_OFFSET = 1469598103934665603ULL;
    const uint64_t FNV_PRIME  = 1099511628211ULL;
    uint64_t h = FNV_OFFSET;
    for (const unsigned char* p = (const unsigned char*)s; *p; ++p) {
        h ^= (uint64_t)(*p);
        h *= FNV_PRIME;
    }
    return h;
}

// ===== Small I/O utility =====
static inline char* read_file_all(const char* path, size_t* out_len) {
    if (out_len) *out_len = 0;
    FILE* f = fopen(path, "rb");
    if (!f) return NULL;
    if (fseek(f, 0, SEEK_END) != 0) { fclose(f); return NULL; }
    long sz = ftell(f);
    if (sz < 0) { fclose(f); return NULL; }
    if (fseek(f, 0, SEEK_SET) != 0) { fclose(f); return NULL; }
    char* buf = (char*)xmalloc((size_t)sz + 1);
    size_t n = fread(buf, 1, (size_t)sz, f);
    buf[n] = '\0';
    fclose(f);
    if (out_len) *out_len = n;
    return buf;
}

#endif 

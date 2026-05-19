#ifndef STRSET_H
#define STRSET_H

#include "common.h"

/* A minimal open-addressing hash set for unique NUL-terminated strings */
typedef struct {
    char **keys;
    size_t cap;
    size_t size;
} StrSet;

/* Initialize with an initial capacity hint  */
void strset_init(StrSet* st, size_t capacity_hint);

/* Inserts string if not present and returns pointer to the stored key */
const char* strset_intern(StrSet* st, const char* key);

/* Export all live keys into caller-provided array */
void strset_export(StrSet* st, char** out);

/* Free the set and all owned strings. */
void strset_free(StrSet* st);

#endif

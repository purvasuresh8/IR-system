#include "strset.h"

static const char* TOMBSTONE = (const char*)(uintptr_t)1;

static size_t next_pow2(size_t x) {
    size_t p = 1;
    while (p < x) p <<= 1;
    return p;
}

static inline uint64_t h64(const char* s) {
    return strhash64(s);
}

static void rehash(StrSet* st) {
    size_t newcap = st->cap ? st->cap << 1 : 16;
    char** newtab = (char**)xcalloc(newcap, sizeof(char*));
    for (size_t i = 0; i < st->cap; ++i) {
        char* k = st->keys[i];
        if (!k || k == TOMBSTONE) continue;
        uint64_t h = h64(k);
        size_t idx = (size_t)(h & (newcap - 1));
        while (newtab[idx]) idx = (idx + 1) & (newcap - 1);
        newtab[idx] = k; /* transfer ownership */
    }
    free(st->keys);
    st->keys = newtab;
    st->cap  = newcap;
}

void strset_init(StrSet* st, size_t capacity_hint) {
    memset(st, 0, sizeof(*st));
    size_t cap = capacity_hint;
    if (cap < 16) cap = 16;
    st->cap = next_pow2(cap);
    st->keys = (char**)xcalloc(st->cap, sizeof(char*));
}

const char* strset_intern(StrSet* st, const char* key) {
    if (st->size * 2 >= st->cap) rehash(st);

    uint64_t h = h64(key);
    size_t idx = (size_t)(h & (st->cap - 1));
    size_t first_tomb = (size_t)-1;

    for (;;) {
        char* cur = st->keys[idx];
        if (!cur) {
            size_t ins = (first_tomb == (size_t)-1) ? idx : first_tomb;
            st->keys[ins] = xstrdup(key);
            st->size++;
            return st->keys[ins];
        }
        if (cur == TOMBSTONE) {
            if (first_tomb == (size_t)-1) first_tomb = idx;
        } else if (strcmp(cur, key) == 0) {
            return cur;
        }
        idx = (idx + 1) & (st->cap - 1);
    }
}

void strset_export(StrSet* st, char** out) {
    size_t k = 0;
    for (size_t i = 0; i < st->cap; ++i) {
        char* cur = st->keys[i];
        if (cur && cur != TOMBSTONE) out[k++] = cur;
    }

}

void strset_free(StrSet* st) {
    if (!st->keys) return;
    for (size_t i = 0; i < st->cap; ++i) {
        char* cur = st->keys[i];
        if (cur && cur != TOMBSTONE) free(cur);
    }
    free(st->keys);
    st->keys = NULL;
    st->cap  = 0;
    st->size = 0;
}

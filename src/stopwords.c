#include "stopwords.h"

static inline bool is_comment_or_blank(const char* p) {
    while (*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n' || *p == '\f' || *p == '\v') ++p;
    if (*p == '\0') return true;
    if (*p == '#')  return true; 
    if (*p == '/' && *(p+1) == '/') return true;
    return false;
}

void load_stopwords(const char* path, StrSet* out) {
    if (out->keys == NULL || out->cap == 0) {
        strset_init(out, 1024);
    }

    FILE* f = fopen(path, "r");
    if (!f) DIE("Cannot open stopword file: %s", path);

    char line[1024];
    while (fgets(line, sizeof(line), f)) {
        str_trim_ascii(line);
        if (is_comment_or_blank(line)) continue;

        str_to_lower_ascii(line);

        (void)strset_intern(out, line);
    }

    fclose(f);
}

bool is_stopword(StrSet* sw, const char* token) {
    if (!sw || !sw->keys || !token || !*token) return false;
    uint64_t h = strhash64(token);
    size_t idx = (size_t)(h & (sw->cap - 1));
    for (;;) {
        char* cur = sw->keys[idx];
        if (!cur) return false;
        if (cur != (char*)(uintptr_t)1 && strcmp(cur, token) == 0) return true;
        idx = (idx + 1) & (sw->cap - 1);
    }
}

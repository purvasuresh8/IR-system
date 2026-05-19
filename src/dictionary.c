#include "dictionary.h"
#include "common.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* ---------------- Internal structures ---------------- */

typedef struct DictEntry {
    char *term;
    int word_id;
    struct DictEntry *next;
} DictEntry;

struct Dictionary {
    DictEntry **table;   /* hash table buckets */
    size_t size;         /* number of buckets */
    int next_id;         /* next wordID to assign */
};

/* ---------------- Hash function ---------------- */

/* djb2 string hash */
static unsigned long hash_string(const char *s) {
    unsigned long h = 5381;
    int c;
    while ((c = *s++))
        h = ((h << 5) + h) + c;
    return h;
}

/* ---------------- API implementation ---------------- */

Dictionary *dictionary_create(size_t table_size)
{
    Dictionary *dict = malloc(sizeof(Dictionary));
    if (!dict) return NULL;

    dict->table = calloc(table_size, sizeof(DictEntry *));
    if (!dict->table) {
        free(dict);
        return NULL;
    }

    dict->size = table_size;
    dict->next_id = 1;  /* wordIDs start at 1 */

    return dict;
}

int dictionary_lookup(Dictionary *dict, const char *term)
{
    unsigned long h = hash_string(term) % dict->size;
    DictEntry *e = dict->table[h];

    while (e) {
        if (strcmp(e->term, term) == 0)
            return e->word_id;
        e = e->next;
    }
    return 0;  /* not found */
}

int dictionary_get_id(Dictionary *dict, const char *term)
{
    unsigned long h = hash_string(term) % dict->size;
    DictEntry *e = dict->table[h];

    /* search existing entries */
    while (e) {
        if (strcmp(e->term, term) == 0)
            return e->word_id;
        e = e->next;
    }

    /* not found → create new entry */
    DictEntry *ne = malloc(sizeof(DictEntry));
    if (!ne) return 0;

    ne->term = strdup(term);
    if (!ne->term) {
        free(ne);
        return 0;
    }

    ne->word_id = dict->next_id++;
    ne->next = dict->table[h];
    dict->table[h] = ne;

    return ne->word_id;
}

static int compare_dict_entries(const void *a, const void *b)
{
    const DictEntry *da = *(const DictEntry **)a;
    const DictEntry *db = *(const DictEntry **)b;

    if (da->word_id < db->word_id) return -1;
    if (da->word_id > db->word_id) return 1;
    return 0;
}

void dictionary_write(Dictionary *dict, const char *path)
{
    FILE *out = fopen(path, "w");
    if (!out) {
        perror("dictionary_write");
        return;
    }

    /* Count entries */
    size_t count = 0;
    for (size_t i = 0; i < dict->size; i++) {
        for (DictEntry *e = dict->table[i]; e; e = e->next) {
            count++;
        }
    }

    /* Collect entries */
    DictEntry **entries = malloc(count * sizeof(DictEntry *));
    if (!entries) DIE("dictionary_write: malloc failed");

    size_t idx = 0;
    for (size_t i = 0; i < dict->size; i++) {
        for (DictEntry *e = dict->table[i]; e; e = e->next) {
            entries[idx++] = e;
        }
    }

    /* Sort by word_id */
    qsort(entries, count, sizeof(DictEntry *), compare_dict_entries);

    /* Write sorted dictionary */
    for (size_t i = 0; i < count; i++) {
        fprintf(out, "%d %s\n", entries[i]->word_id, entries[i]->term);
    }

    free(entries);
    fclose(out);
}

void dictionary_free(Dictionary *dict)
{
    if (!dict) return;

    for (size_t i = 0; i < dict->size; i++) {
        DictEntry *e = dict->table[i];
        while (e) {
            DictEntry *next = e->next;
            free(e->term);
            free(e);
            e = next;
        }
    }

    free(dict->table);
    free(dict);
}

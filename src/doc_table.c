#include "doc_table.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/* ---------------- Internal structures ---------------- */

/* One document table entry (hash table bucket) */
typedef struct DocEntry {
    char *docno;
    int doc_id;
    struct DocEntry *next;
} DocEntry;

struct DocTable {
    DocEntry **table;        /* hash table: DOCNO -> docID */
    size_t table_size;
    int next_id;             /* next docID to assign (starts at 1) */

    /* Reverse lookup for Project 3 */
    char **id_to_docno;      /* docID -> DOCNO */
};

/* ---------------- Helper functions ---------------- */

static unsigned long hash_docno(const char *s)
{
    unsigned long hash = 5381;
    int c;

    while ((c = *s++))
        hash = ((hash << 5) + hash) + c; /* djb2 */

    return hash;
}

/* ---------------- API implementation ---------------- */

DocTable *doc_table_create(size_t table_size)
{
    DocTable *dt = malloc(sizeof(DocTable));
    if (!dt) return NULL;

    dt->table_size = table_size;
    dt->table = calloc(table_size, sizeof(DocEntry *));
    if (!dt->table) {
        free(dt);
        return NULL;
    }

    dt->next_id = 1;

    /* Allocate reverse lookup array (docIDs start at 1) */
    dt->id_to_docno = calloc(table_size + 1, sizeof(char *));
    if (!dt->id_to_docno) {
        free(dt->table);
        free(dt);
        return NULL;
    }

    return dt;
}

int doc_table_get_id(DocTable *dt, const char *docno)
{
    if (!dt || !docno) return 0;

    unsigned long h = hash_docno(docno) % dt->table_size;
    DocEntry *e = dt->table[h];

    while (e) {
        if (strcmp(e->docno, docno) == 0)
            return e->doc_id;
        e = e->next;
    }

    /* New document */
    DocEntry *new_entry = malloc(sizeof(DocEntry));
    if (!new_entry) return 0;

    new_entry->docno = strdup(docno);
    new_entry->doc_id = dt->next_id++;
    new_entry->next = dt->table[h];
    dt->table[h] = new_entry;

    /* Store reverse mapping */
    dt->id_to_docno[new_entry->doc_id] = new_entry->docno;

    return new_entry->doc_id;
}

int doc_table_lookup(DocTable *dt, const char *docno)
{
    if (!dt || !docno) return 0;

    unsigned long h = hash_docno(docno) % dt->table_size;
    DocEntry *e = dt->table[h];

    while (e) {
        if (strcmp(e->docno, docno) == 0)
            return e->doc_id;
        e = e->next;
    }

    return 0;
}

void doc_table_write(DocTable *dt, const char *path)
{
    if (!dt || !path) return;

    FILE *out = fopen(path, "w");
    if (!out) {
        perror("doc_table_write");
        return;
    }

    for (int i = 1; i < dt->next_id; i++) {
        fprintf(out, "%d %s\n", i, dt->id_to_docno[i]);
    }

    fclose(out);
}

void doc_table_free(DocTable *dt)
{
    if (!dt) return;

    for (size_t i = 0; i < dt->table_size; i++) {
        DocEntry *e = dt->table[i];
        while (e) {
            DocEntry *next = e->next;
            free(e->docno);
            free(e);
            e = next;
        }
    }

    free(dt->id_to_docno);
    free(dt->table);
    free(dt);
}

/* ---------------- Project 3 accessors ---------------- */

int doc_table_size(DocTable *dt)
{
    if (!dt) return 0;
    return dt->next_id - 1;
}

const char *doc_table_get_docno(DocTable *dt, int doc_id)
{
    if (!dt || doc_id <= 0 || doc_id >= dt->next_id)
        return NULL;

    return dt->id_to_docno[doc_id];
}



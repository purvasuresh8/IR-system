#ifndef DOC_TABLE_H
#define DOC_TABLE_H

#include <stddef.h>

/*
 * Document Table
 *
 * Maps document identifiers (DOCNO strings from TREC/FT)
 * to unique integer document IDs (docID).
 *
 * docIDs start from 1 and increase by 1 for each new document.
 */

typedef struct DocTable DocTable;

DocTable *doc_table_create(size_t table_size);

int doc_table_get_id(DocTable *dt, const char *docno);

int doc_table_lookup(DocTable *dt, const char *docno);

void doc_table_write(DocTable *dt, const char *path);

void doc_table_free(DocTable *dt);

/* ======== NEW FOR PROJECT 3 ======== */

/* Total number of documents */
int doc_table_size(DocTable *dt);

/* Get DOCNO string from a docID */
const char *doc_table_get_docno(DocTable *dt, int doc_id);

#endif /* DOC_TABLE_H */

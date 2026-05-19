#ifndef INDEXER_H
#define INDEXER_H

#include "dictionary.h"
#include "doc_table.h"
#include "forward_index.h"
#include "inverted_index.h"
#include "strset.h"

/*
 * Indexer
 *
 * Coordinates parsing, dictionary construction,
 * forward index, and inverted index creation.
 */

/* Opaque indexer type */
typedef struct Indexer Indexer;

/*
 * Create a new indexer and initialize all substructures.
 */
Indexer *indexer_create(StrSet *stopwords);

/*
 * Build all indexes for a given FT/TREC root directory.
 */
void indexer_build(Indexer *idx, const char *ft_root);

/*
 * Write all index files to disk.
 */
void indexer_write(Indexer *idx);

/*
 * Free all memory associated with the indexer.
 */
void indexer_free(Indexer *idx);


Dictionary *indexer_get_dictionary(Indexer *idx);
DocTable *indexer_get_doc_table(Indexer *idx);
InvertedIndex *indexer_get_inverted_index(Indexer *idx);

#endif

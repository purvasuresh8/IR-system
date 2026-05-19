#ifndef INVERTED_INDEX_H
#define INVERTED_INDEX_H

#include <stddef.h>

/*
 * Inverted Index
 *
 * Stores, for each wordID, the list of documents (docIDs)
 * in which the word appears along with its frequency.
 */

typedef struct InvertedIndex InvertedIndex;

/* Create a new empty inverted index */
InvertedIndex *inverted_index_create(void);

/* Add a posting */
void inverted_index_add(InvertedIndex *ii,
                        int word_id,
                        int doc_id,
                        int freq);

/* Write inverted index to disk */
void inverted_index_write(InvertedIndex *ii, const char *path);

/* Free inverted index */
void inverted_index_free(InvertedIndex *ii);

/* ======== NEW FOR PROJECT 3 ======== */

/* Document frequency for a word */
int inverted_index_df(InvertedIndex *ii, int word_id);

/* Read-only access to postings */
int inverted_index_get_postings(InvertedIndex *ii,
                                int word_id,
                                int **doc_ids,
                                int **freqs,
                                int *count);

#endif

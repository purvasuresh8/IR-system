#ifndef FORWARD_INDEX_H
#define FORWARD_INDEX_H

#include <stddef.h>

/*
 * Forward Index
 *
 * Stores, for each document (docID), the list of terms (wordIDs)
 * that occur in the document along with their frequencies.
 *
 * Conceptually:
 *   docID -> (wordID : frequency)
 */

/* Opaque forward index type */
typedef struct ForwardIndex ForwardIndex;

/*
 * Create a new empty forward index.
 */
ForwardIndex *forward_index_create(void);

/*
 * Add one occurrence of a wordID to a document.
 * If the wordID already exists for the docID, its
 * frequency is incremented.
 */
void forward_index_add(ForwardIndex *fi, int doc_id, int word_id);

/*
 * Write the forward index to disk.
 * Format (per line):
 *   docID: wordID:freq; wordID:freq; ...
 */
void forward_index_write(ForwardIndex *fi, const char *path);

/*
 * Free all memory associated with the forward index.
 */
void forward_index_free(ForwardIndex *fi);

#endif /* FORWARD_INDEX_H */

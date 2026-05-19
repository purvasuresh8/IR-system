#ifndef DICTIONARY_H
#define DICTIONARY_H

#include <stddef.h>

/*
 * Dictionary (Lexicon)
 *
 * Maps unique terms (strings) to unique integer wordIDs.
 * WordIDs start from 1 and increase by 1 for each new term.
 *
 * Used by:
 *   - Forward index (docID -> wordID:freq)
 *   - Inverted index (wordID -> docID:freq)
 */

/* Forward declaration */
typedef struct Dictionary Dictionary;

/*
 * Create a dictionary with a given hash table size.
 * Typical size: 100003 (prime) for TREC-scale collections.
 */
Dictionary *dictionary_create(size_t table_size);

/*
 * Get the wordID for a term.
 * - If the term already exists, returns its existing wordID
 * - If the term is new, assigns a new wordID and returns it
 */
int dictionary_get_id(Dictionary *dict, const char *term);

/*
 * Look up a term without inserting.
 * Returns:
 *   - wordID if term exists
 *   - 0 if term does not exist
 */
int dictionary_lookup(Dictionary *dict, const char *term);

/*
 * Write the dictionary to disk.
 * Format:
 *   wordID term
 */
void dictionary_write(Dictionary *dict, const char *path);

/*
 * Free all memory associated with the dictionary.
 */
void dictionary_free(Dictionary *dict);

#endif /* DICTIONARY_H */

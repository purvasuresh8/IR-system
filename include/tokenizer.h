#ifndef TOKENIZER_H
#define TOKENIZER_H

#include <stddef.h>
#include "strset.h"

/*
 * Tokenize, normalize, remove stopwords, and apply Porter stemming.
 *
 * Parameters:
 *   text      - input text to process
 *   stopwords - set of stopwords to remove
 *   count     - output parameter set to number of tokens returned
 *
 * Returns:
 *   A heap-allocated array of heap-allocated C-strings.
 *
 * Ownership:
 *   Caller must free each string in the array, then free the array itself.
 */
char **tokenize_and_stem(const char *text, StrSet *stopwords, size_t *count);

/*
 * Apply Porter stemming to a single token in-place.
 */
void porter_stem_inplace(char *s);

#endif /* TOKENIZER_H */


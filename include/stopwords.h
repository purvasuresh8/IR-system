#ifndef STOPWORDS_H
#define STOPWORDS_H

#include "common.h"
#include "strset.h"

/* Load a stopword list (one term per line) into a StrSet */
void load_stopwords(const char* path, StrSet* out);

/* Return true if token is a stopword */
bool is_stopword(StrSet* sw, const char* token);

#endif

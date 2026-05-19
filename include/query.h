#ifndef QUERY_H
#define QUERY_H

#include "inverted_index.h"
#include "doc_table.h"
#include "dictionary.h"
#include "strset.h"

/* Project 3 entry point */
void process_queries(InvertedIndex *index,
                     DocTable *doc_table,
                     Dictionary *dict,
                     StrSet *stopwords);

#endif

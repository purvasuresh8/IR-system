#include "indexer.h"
#include "ft_parser.h"

#include <stdlib.h>
#include <stdio.h>

/* ------------------------------------------------ */
/* Indexer structure                               */
/* ------------------------------------------------ */

struct Indexer {
    Dictionary    *dict;
    DocTable      *docs;
    ForwardIndex  *fwd;
    InvertedIndex *inv;
    StrSet        *stopwords;
};

/* ------------------------------------------------ */
/* Parser callback                                 */
/* ------------------------------------------------ */

static void index_document(const char *docno,
                           char **tokens,
                           size_t tok_count,
                           void *user)
{
    Indexer *idx = (Indexer *)user;

    static size_t doc_count = 0;
    doc_count++;

    int doc_id = doc_table_get_id(idx->docs, docno);

    for (size_t i = 0; i < tok_count; i++) {
        int word_id = dictionary_get_id(idx->dict, tokens[i]);
        forward_index_add(idx->fwd, doc_id, word_id);
        inverted_index_add(idx->inv, word_id, doc_id, 1);
    }

    /* Progress update every 1000 documents */
    if (doc_count % 1000 == 0) {
        fprintf(stderr, "Indexed %zu documents so far\n", doc_count);
    }

    /* Token cleanup */
    for (size_t i = 0; i < tok_count; i++) {
        free(tokens[i]);
    }
    free(tokens);
}

/* ------------------------------------------------ */
/* Public API                                      */
/* ------------------------------------------------ */

Indexer *indexer_create(StrSet *stopwords)
{
    Indexer *idx = malloc(sizeof(Indexer));
    if (!idx) return NULL;

    idx->dict = dictionary_create(100003);
    idx->docs = doc_table_create(100003);
    idx->fwd  = forward_index_create();
    idx->inv  = inverted_index_create();
    idx->stopwords = stopwords;

    return idx;
}

void indexer_build(Indexer *idx, const char *ft_root)
{
    ft_parse_for_indexing(ft_root,
                          index_document,
                          idx,
                          idx->stopwords);
}

void indexer_write(Indexer *idx)
{
    dictionary_write(idx->dict, "output/dictionary.txt");
    doc_table_write(idx->docs, "output/doc_table.txt");
    forward_index_write(idx->fwd, "output/forward_index.txt");
    inverted_index_write(idx->inv, "output/inverted_index.txt");
}

void indexer_free(Indexer *idx)
{
    if (!idx) return;

    dictionary_free(idx->dict);
    doc_table_free(idx->docs);
    forward_index_free(idx->fwd);
    inverted_index_free(idx->inv);

    free(idx);
}

Dictionary *indexer_get_dictionary(Indexer *idx)
{
    return idx ? idx->dict : NULL;
}

DocTable *indexer_get_doc_table(Indexer *idx)
{
    return idx ? idx->docs : NULL;
}

InvertedIndex *indexer_get_inverted_index(Indexer *idx)
{
    return idx ? idx->inv : NULL;
}


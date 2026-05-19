#ifndef FT_PARSER_H
#define FT_PARSER_H
#include <stddef.h>
#include "strset.h"

#include "common.h"

void ft_parse_for_indexing(
    const char *root,
    void (*on_doc) (const char *docno, char **tokens, size_t tok_count,
         void *user), void *user, StrSet *stopwords
    );
typedef struct {
    char*  docno;
    char** tokens;
    size_t tok_count;
    size_t tok_cap;
} DocRec;

typedef struct {
    DocRec* arr;
    size_t  len;
    size_t  cap;
} DocVec;

typedef struct {
    char **tokens;
    int count;
} TokenList;

void   docvec_init(DocVec* dv);
DocRec* docvec_add(DocVec* dv, const char* docno);
void   docvec_free(DocVec* dv);

TokenList *parse_document (const char* doc_path);
void free_tokens(TokenList *list);


/* A callback invoked for each parsed document.
   - docno
   - text
   - user
*/
typedef void (*doc_callback)(const char* docno, const char* text, void* user);

void ft_walk_and_parse(const char* ft_root, doc_callback cb, void* user);

#endif

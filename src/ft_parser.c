#include "ft_parser.h"

#include "tokenizer.h"
#include "strset.h"
#include "common.h"

#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

/* ------------------------------------------------ */
/* Helpers                                          */
/* ------------------------------------------------ */

static char *strcasestr_simple(const char *hay, const char *needle) {
    if (!*needle) return (char *)hay;
    size_t nh = strlen(hay), nn = strlen(needle);
    for (size_t i = 0; i + nn <= nh; i++) {
        if (strncasecmp(hay + i, needle, nn) == 0)
            return (char *)(hay + i);
    }
    return NULL;
}

static void strip_tags_inplace(char *s) {
    char *w = s;
    for (char *r = s; *r; r++) {
        if (*r == '<') {
            while (*r && *r != '>') r++;
            continue;
        }
        *w++ = *r;
    }
    *w = '\0';
}

/* ------------------------------------------------ */
/* FT document parsing                              */
/* ------------------------------------------------ */

static void parse_ft_documents(const char *content,
                               doc_callback cb,
                               void *user)
{
    const char *p = content;

    while ((p = strcasestr_simple(p, "<DOC>"))) {
        const char *end = strcasestr_simple(p, "</DOC>");
        if (!end) break;

        char *block = strndup(p, end - p);

        char *n1 = strcasestr_simple(block, "<DOCNO>");
        char *n2 = n1 ? strcasestr_simple(n1, "</DOCNO>") : NULL;

        if (n1 && n2) {
            n1 += strlen("<DOCNO>");
            char *docno = strndup(n1, n2 - n1);
            str_trim_ascii(docno);

            size_t cap = 1024, len = 0;
            char *text = xmalloc(cap);
            text[0] = '\0';

            char *q = block;
            while ((q = strcasestr_simple(q, "<TEXT>"))) {
                char *qe = strcasestr_simple(q, "</TEXT>");
                if (!qe) break;

                q += strlen("<TEXT>");
                char *piece = strndup(q, qe - q);

                strip_tags_inplace(piece);
                str_trim_ascii(piece);

                size_t plen = strlen(piece);
                if (plen) {
                    while (len + plen + 2 > cap)
                        cap *= 2;

                    text = xrealloc(text, cap);
                    memcpy(text + len, piece, plen);
                    len += plen;
                    text[len++] = ' ';
                    text[len] = '\0';
                }

                free(piece);
                q = qe + strlen("</TEXT>");
            }

            cb(docno, text, user);

            free(docno);
            free(text);
        }

        free(block);
        p = end + strlen("</DOC>");
    }
}

/* ------------------------------------------------ */
/* Directory traversal                              */
/* ------------------------------------------------ */

static void walk_dir(const char *root,
                     doc_callback cb,
                     void *user)
{
    DIR *d = opendir(root);
    if (!d) DIE("Cannot open FT root: %s", root);

    struct dirent *e;
    while ((e = readdir(d))) {
        if (!strcmp(e->d_name, ".") || !strcmp(e->d_name, ".."))
            continue;

        char path[4096];
        snprintf(path, sizeof(path), "%s/%s", root, e->d_name);

        struct stat st;
        if (stat(path, &st) != 0) continue;

        if (S_ISDIR(st.st_mode)) {
            walk_dir(path, cb, user);
        } else {
            const char *ext = strrchr(e->d_name, '.');
            if (!ext) continue;

            if (strcasecmp(ext, ".sgm") &&
                strcasecmp(ext, ".sgml") &&
                strcasecmp(ext, ".txt"))
                continue;

            char *content = read_file_all(path, NULL);
            if (content) {
                parse_ft_documents(content, cb, user);
                free(content);
            }
        }
    }
    closedir(d);
}

void ft_walk_and_parse(const char *root,
                       doc_callback cb,
                       void *user)
{
    if (!root || !cb)
        DIE("ft_walk_and_parse: invalid arguments");

    walk_dir(root, cb, user);
}

/* ================================================= */
/* Token export adapter (Parser → Indexer)           */
/* ================================================= */

typedef struct {
    void (*on_doc)(const char *docno,
                   char **tokens,
                   size_t tok_count,
                   void *user);
    void *user;
    StrSet *stopwords;
} IndexerBridge;

/* ✅ FIXED: correct stopwords + count usage */
static void indexer_adapter_cb(const char *docno,
                               const char *text,
                               void *u)
{
    IndexerBridge *b = (IndexerBridge *)u;

    size_t tok_count = 0;
    char **tokens = tokenize_and_stem(text, b->stopwords, &tok_count);

    /* Token ownership transfers to the indexer */
    b->on_doc(docno, tokens, tok_count, b->user);
}

void ft_parse_for_indexing(const char *root,
                           void (*on_doc)(const char *docno,
                                          char **tokens,
                                          size_t tok_count,
                                          void *user),
                           void *user,
                           StrSet *stopwords)
{
    IndexerBridge bridge = {
        .on_doc = on_doc,
        .user = user,
        .stopwords = stopwords
    };

    ft_walk_and_parse(root, indexer_adapter_cb, &bridge);
}

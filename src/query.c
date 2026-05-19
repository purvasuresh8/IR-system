#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "query.h"
#include "tokenizer.h"
#include "dictionary.h"
#include "inverted_index.h"
#include "doc_table.h"
#include "common.h"

/* ---------------- Data structures ---------------- */

typedef struct QueryTerm {
    int word_id;
    int tf;
    struct QueryTerm *next;
} QueryTerm;

typedef struct {
    int doc_id;
    double score;
} DocScore;

/* ---------------- Utilities ---------------- */

static void add_query_term(QueryTerm **head, int word_id)
{
    for (QueryTerm *q = *head; q; q = q->next) {
        if (q->word_id == word_id) {
            q->tf++;
            return;
        }
    }

    QueryTerm *n = malloc(sizeof(QueryTerm));
    if (!n) DIE("malloc failed");

    n->word_id = word_id;
    n->tf = 1;
    n->next = *head;
    *head = n;
}

static int cmp_scores(const void *a, const void *b)
{
    const DocScore *x = a;
    const DocScore *y = b;

    if (y->score > x->score) return 1;
    if (y->score < x->score) return -1;
    return 0;
}

/* ---------------- Build query vector ---------------- */

static QueryTerm *build_query(const char *text,
                              Dictionary *dict,
                              StrSet *stopwords)
{
    QueryTerm *terms = NULL;
    size_t count = 0;

    char **tokens = tokenize_and_stem(text, stopwords, &count);

    for (size_t i = 0; i < count; i++) {
        int wid = dictionary_lookup(dict, tokens[i]);
        if (wid > 0)
            add_query_term(&terms, wid);
        free(tokens[i]);
    }
    free(tokens);

    return terms;
}

/* ---------------- Score documents ---------------- */

static int score_documents(QueryTerm *qt,
                           InvertedIndex *index,
                           DocTable *doc_table,
                           DocScore **out_scores)
{
    int N = doc_table_size(doc_table);

    /* Find max doc_id used */
    int max_doc_id = 0;
    for (QueryTerm *q = qt; q; q = q->next) {
        int *docs, *freqs, count;
        if (!inverted_index_get_postings(index,
                                         q->word_id,
                                         &docs, &freqs, &count))
            continue;
        for (int i = 0; i < count; i++)
            if (docs[i] > max_doc_id)
                max_doc_id = docs[i];
    }

    if (max_doc_id == 0) {
        *out_scores = NULL;
        return 0;
    }

    double *doc_scores = calloc(max_doc_id + 1, sizeof(double));
    double *doc_norms  = calloc(max_doc_id + 1, sizeof(double));
    if (!doc_scores || !doc_norms)
        DIE("calloc failed");

    double query_norm = 0.0;

    for (QueryTerm *q = qt; q; q = q->next) {
        int df = inverted_index_df(index, q->word_id);
        if (df == 0)
            continue;

        double idf = log((double)N / df);
        double wq = q->tf * idf;
        query_norm += wq * wq;

        int *docs, *freqs, count;
        if (!inverted_index_get_postings(index,
                                         q->word_id,
                                         &docs, &freqs, &count))
            continue;

        for (int i = 0; i < count; i++) {
            int d = docs[i];
            double wd = freqs[i] * idf;
            doc_scores[d] += wq * wd;
            doc_norms[d]  += wd * wd;
        }
    }

    query_norm = sqrt(query_norm);
    if (query_norm == 0.0) {
        free(doc_scores);
        free(doc_norms);
        *out_scores = NULL;
        return 0;
    }

    DocScore *scores = malloc(max_doc_id * sizeof(DocScore));
    if (!scores) DIE("malloc failed");

    int n_scores = 0;
    for (int d = 1; d <= max_doc_id; d++) {
        if (doc_scores[d] > 0.0 && doc_norms[d] > 0.0) {
            scores[n_scores].doc_id = d;
            scores[n_scores].score =
                doc_scores[d] / (query_norm * sqrt(doc_norms[d]));
            n_scores++;
        }
    }

    free(doc_scores);
    free(doc_norms);

    qsort(scores, n_scores, sizeof(DocScore), cmp_scores);

    *out_scores = scores;
    return n_scores;
}

/* ---------------- Robust TREC parsing ---------------- */

static int read_query(FILE *fp,
                      int *topic,
                      char *title,
                      char *desc,
                      char *narr)
{
    char line[4096];
    *topic = 0;
    title[0] = desc[0] = narr[0] = '\0';

    while (fgets(line, sizeof(line), fp)) {
        if (strstr(line, "<num>")) {
            sscanf(line, "%*[^0-9]%d", topic);
            break;
        }
    }
    if (*topic == 0) return 0;

    while (fgets(line, sizeof(line), fp))
        if (strstr(line, "<title>")) break;
    fgets(title, 2048, fp);
    str_trim_ascii(title);

    while (fgets(line, sizeof(line), fp))
        if (strstr(line, "<desc>")) break;
    while (fgets(line, sizeof(line), fp)) {
        if (strstr(line, "<narr>")) break;
        strcat(desc, line);
    }
    str_trim_ascii(desc);

    while (fgets(line, sizeof(line), fp)) {
        if (strstr(line, "</top>")) break;
        strcat(narr, line);
    }
    str_trim_ascii(narr);

    return 1;
}

/* ---------------- Entry point ---------------- */

void process_queries(InvertedIndex *index,
                     DocTable *doc_table,
                     Dictionary *dict,
                     StrSet *stopwords)
{
    FILE *in = fopen("topics.txt", "r");
    if (!in) DIE("cannot open topics.txt");

    FILE *out_title = fopen("output/vsm_title.txt", "w");
    FILE *out_td    = fopen("output/vsm_title_desc.txt", "w");
    FILE *out_tn    = fopen("output/vsm_title_narr.txt", "w");
    if (!out_title || !out_td || !out_tn)
        DIE("output file error");

    int topic;
    char title[2048], desc[2048], narr[2048];
    char buf[6144];

    while (read_query(in, &topic, title, desc, narr)) {

        /* -------- TITLE ONLY -------- */
        QueryTerm *qt = build_query(title, dict, stopwords);
        DocScore *scores;
        int n = score_documents(qt, index, doc_table, &scores);

        /* Fallback: if title-only is empty, use title + description */
        if (n == 0) {
            char fallback[4096];
            snprintf(fallback, sizeof(fallback), "%s %s", title, desc);
            qt = build_query(fallback, dict, stopwords);
             n = score_documents(qt, index, doc_table, &scores);
            }
        for (int i = 0; i < n; i++)
            fprintf(out_title, "%d\t%s\t%d\t%f\n",
                    topic,
                    doc_table_get_docno(doc_table, scores[i].doc_id),
                    i + 1,
                    scores[i].score);
        free(scores);

        /* -------- TITLE + DESCRIPTION -------- */
        snprintf(buf, sizeof(buf), "%s %s", title, desc);
        qt = build_query(buf, dict, stopwords);
        n = score_documents(qt, index, doc_table, &scores);
        for (int i = 0; i < n; i++)
            fprintf(out_td, "%d\t%s\t%d\t%f\n",
                    topic,
                    doc_table_get_docno(doc_table, scores[i].doc_id),
                    i + 1,
                    scores[i].score);
        free(scores);

        /* -------- TITLE + NARRATIVE -------- */
        snprintf(buf, sizeof(buf), "%s %s", title, narr);
        qt = build_query(buf, dict, stopwords);
        n = score_documents(qt, index, doc_table, &scores);
        for (int i = 0; i < n; i++)
            fprintf(out_tn, "%d\t%s\t%d\t%f\n",
                    topic,
                    doc_table_get_docno(doc_table, scores[i].doc_id),
                    i + 1,
                    scores[i].score);
        free(scores);
    }

    fclose(in);
    fclose(out_title);
    fclose(out_td);
    fclose(out_tn);
}


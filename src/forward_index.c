#include "forward_index.h"

#include <stdlib.h>
#include <stdio.h>

/* ---------------- Internal structures ---------------- */

/*
 * Stores frequency of a word in a document
 */
typedef struct WordFreq {
    int word_id;
    int freq;
    struct WordFreq *next;
} WordFreq;

/*
 * Stores all word frequencies for a document
 */
typedef struct DocEntry {
    int doc_id;
    WordFreq *words;
    struct DocEntry *next;
} DocEntry;

struct ForwardIndex {
    DocEntry *docs;   /* linked list of documents */
};

/* ---------------- Helper functions ---------------- */

static DocEntry *find_doc(DocEntry *docs, int doc_id)
{
    while (docs) {
        if (docs->doc_id == doc_id)
            return docs;
        docs = docs->next;
    }
    return NULL;
}

static WordFreq *find_word(WordFreq *wf, int word_id)
{
    while (wf) {
        if (wf->word_id == word_id)
            return wf;
        wf = wf->next;
    }
    return NULL;
}

/* ---------------- API implementation ---------------- */

ForwardIndex *forward_index_create(void)
{
    ForwardIndex *fi = malloc(sizeof(ForwardIndex));
    if (!fi) return NULL;

    fi->docs = NULL;
    return fi;
}

void forward_index_add(ForwardIndex *fi, int doc_id, int word_id)
{
    if (!fi) return;

    /* find or create document entry */
    DocEntry *doc = find_doc(fi->docs, doc_id);
    if (!doc) {
        doc = malloc(sizeof(DocEntry));
        if (!doc) return;

        doc->doc_id = doc_id;
        doc->words = NULL;
        doc->next = fi->docs;
        fi->docs = doc;
    }

    /* find or create word entry */
    WordFreq *wf = find_word(doc->words, word_id);
    if (wf) {
        wf->freq++;
    } else {
        wf = malloc(sizeof(WordFreq));
        if (!wf) return;

        wf->word_id = word_id;
        wf->freq = 1;
        wf->next = doc->words;
        doc->words = wf;
    }
}

void forward_index_write(ForwardIndex *fi, const char *path)
{
    FILE *out = fopen(path, "w");
    if (!out) {
        perror("forward_index_write");
        return;
    }

    for (DocEntry *d = fi->docs; d != NULL; d = d->next) {
        fprintf(out, "%d:", d->doc_id);

        for (WordFreq *w = d->words; w != NULL; w = w->next) {
            fprintf(out, " %d:%d;", w->word_id, w->freq);
        }
        fprintf(out, "\n");
    }

    fclose(out);
}

void forward_index_free(ForwardIndex *fi)
{
    if (!fi) return;

    DocEntry *d = fi->docs;
    while (d) {
        WordFreq *w = d->words;
        while (w) {
            WordFreq *next_w = w->next;
            free(w);
            w = next_w;
        }
        DocEntry *next_d = d->next;
        free(d);
        d = next_d;
    }

    free(fi);
}

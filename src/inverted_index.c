#include "inverted_index.h"

#include <stdlib.h>
#include <stdio.h>

/* ---------------- Internal structures ---------------- */

/*
 * One posting: a document and the term frequency in that document
 */
typedef struct Posting {
    int doc_id;
    int freq;
    struct Posting *next;
} Posting;

/*
 * One inverted list entry: all postings for a single word
 */
typedef struct WordEntry {
    int word_id;
    Posting *postings;
    struct WordEntry *next;
} WordEntry;

struct InvertedIndex {
    WordEntry *words;    /* sorted linked list of word entries */
};

/* ---------------- Helper functions ---------------- */

/* Find a word entry */
static WordEntry *find_word(WordEntry *words, int word_id)
{
    while (words) {
        if (words->word_id == word_id)
            return words;
        words = words->next;
    }
    return NULL;
}

/* Insert word entry in sorted order if it does not exist */
static WordEntry *insert_word_sorted(WordEntry **head, int word_id)
{
    WordEntry *prev = NULL;
    WordEntry *curr = *head;

    while (curr && curr->word_id < word_id) {
        prev = curr;
        curr = curr->next;
    }

    if (curr && curr->word_id == word_id) {
        return curr; /* already exists */
    }

    WordEntry *we = malloc(sizeof(WordEntry));
    if (!we) return NULL;

    we->word_id = word_id;
    we->postings = NULL;
    we->next = curr;

    if (prev)
        prev->next = we;
    else
        *head = we;

    return we;
}

/* Insert or update posting in sorted order by doc_id */
static void insert_posting_sorted(Posting **head, int doc_id, int freq)
{
    Posting *prev = NULL;
    Posting *curr = *head;

    while (curr && curr->doc_id < doc_id) {
        prev = curr;
        curr = curr->next;
    }

    if (curr && curr->doc_id == doc_id) {
        /* Merge frequencies */
        curr->freq += freq;
        return;
    }

    Posting *p = malloc(sizeof(Posting));
    if (!p) return;

    p->doc_id = doc_id;
    p->freq = freq;
    p->next = curr;

    if (prev)
        prev->next = p;
    else
        *head = p;
}

/* ---------------- API implementation ---------------- */

InvertedIndex *inverted_index_create(void)
{
    InvertedIndex *ii = malloc(sizeof(InvertedIndex));
    if (!ii) return NULL;

    ii->words = NULL;
    return ii;
}

void inverted_index_add(InvertedIndex *ii,
                        int word_id,
                        int doc_id,
                        int freq)
{
    if (!ii) return;
    if (word_id < 0 || doc_id < 0 || freq <= 0) return;

    /* find or create word entry (sorted) */
    WordEntry *we = insert_word_sorted(&ii->words, word_id);
    if (!we) return;

    /* insert or merge posting (sorted) */
    insert_posting_sorted(&we->postings, doc_id, freq);
}

void inverted_index_write(InvertedIndex *ii, const char *path)
{
    if (!ii || !path) return;

    FILE *out = fopen(path, "w");
    if (!out) {
        perror("inverted_index_write");
        return;
    }

    for (WordEntry *w = ii->words; w != NULL; w = w->next) {
        fprintf(out, "%d:", w->word_id);

        for (Posting *p = w->postings; p != NULL; p = p->next) {
            fprintf(out, " %d:%d;", p->doc_id, p->freq);
        }
        fprintf(out, "\n");
    }

    fclose(out);
}

void inverted_index_free(InvertedIndex *ii)
{
    if (!ii) return;

    WordEntry *w = ii->words;
    while (w) {
        Posting *p = w->postings;
        while (p) {
            Posting *next_p = p->next;
            free(p);
            p = next_p;
        }
        WordEntry *next_w = w->next;
        free(w);
        w = next_w;
    }

    free(ii);
}

/* ================= Project 3 Accessors ================= */

int inverted_index_df(InvertedIndex *ii, int word_id)
{
    if (!ii) return 0;

    WordEntry *we = find_word(ii->words, word_id);
    if (!we) return 0;

    int df = 0;
    for (Posting *p = we->postings; p; p = p->next)
        df++;

    return df;
}

int inverted_index_get_postings(InvertedIndex *ii,
                                int word_id,
                                int **doc_ids,
                                int **freqs,
                                int *count)
{
    if (!ii || !doc_ids || !freqs || !count)
        return 0;

    WordEntry *we = find_word(ii->words, word_id);
    if (!we)
        return 0;

    int n = 0;
    for (Posting *p = we->postings; p; p = p->next)
        n++;

    if (n == 0)
        return 0;

    static int *docs = NULL;
    static int *fs   = NULL;

    docs = realloc(docs, n * sizeof(int));
    fs   = realloc(fs,   n * sizeof(int));

    int i = 0;
    for (Posting *p = we->postings; p; p = p->next) {
        docs[i] = p->doc_id;
        fs[i]   = p->freq;
        i++;
    }

    *doc_ids = docs;
    *freqs   = fs;
    *count   = n;

    return 1;
}


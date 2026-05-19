#include "indexer.h"
#include "strset.h"
#include "stopwords.h"
#include "common.h"
#include "query.h"

#include <stdio.h>
#include <stdlib.h>

static void usage(const char *prog) {
    fprintf(stderr,
        "Usage: %s <FT_ROOT> <STOPWORDS>\n"
        "  FT_ROOT   : Root directory of FT/TREC collection\n"
        "  STOPWORDS : Stopword list file\n",
        prog);
    exit(1);
}

int main(int argc, char **argv) {
    if (argc != 3) {
        usage(argv[0]);
    }

    fprintf(stderr, "STARTING INDEXER\n");

    const char *ft_root = argv[1];
    const char *sw_file = argv[2];

    /* Load stopwords */
    StrSet stopwords;
    strset_init(&stopwords, 1024);
    load_stopwords(sw_file, &stopwords);

    /* Build indexes */
    Indexer *idx = indexer_create(&stopwords);
    if (!idx) DIE("Failed to create indexer");

    indexer_build(idx, ft_root);
    indexer_write(idx);

    fprintf(stderr, "[OK] Indexing completed\n");

process_queries(
    indexer_get_inverted_index(idx),
    indexer_get_doc_table(idx),
    indexer_get_dictionary(idx),
    &stopwords
);

    /* Cleanup */
    indexer_free(idx);
    strset_free(&stopwords);

    return 0;
}

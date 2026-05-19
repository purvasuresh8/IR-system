#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "tokenizer.h"
#include "common.h"

/* Initial capacity for token array */
#define INIT_TOKEN_CAP 32
#define MAX_TOKEN_LEN 256

/* Normalize token:
 *  - lowercase
 *  - keep alphanumeric characters only
 */
static void normalize_token(const char *src, char *dst)
{
    size_t j = 0;
    for (size_t i = 0; src[i] != '\0' && j < MAX_TOKEN_LEN - 1; i++) {
        if (isalnum((unsigned char)src[i])) {
            dst[j++] = tolower((unsigned char)src[i]);
        }
    }
    dst[j] = '\0';
}

char **tokenize_and_stem(const char *text,
                         StrSet *stopwords,   /* unused for Phase 3 */
                         size_t *count)
{
    if (!count) DIE("tokenize_and_stem: count is NULL");

    size_t capacity = INIT_TOKEN_CAP;
    size_t n = 0;

    char **tokens = malloc(capacity * sizeof(char *));
    if (!tokens) DIE("malloc failed");

    char buffer[MAX_TOKEN_LEN];
    size_t buf_len = 0;

    for (const char *p = text; ; p++) {

        if (*p && isalnum((unsigned char)*p)) {
            if (buf_len < MAX_TOKEN_LEN - 1)
                buffer[buf_len++] = *p;
        } else {
            if (buf_len > 0) {
                buffer[buf_len] = '\0';

                char normalized[MAX_TOKEN_LEN];
                normalize_token(buffer, normalized);

                if (normalized[0] != '\0') {
                    if (n == capacity) {
                        capacity *= 2;
                        tokens = realloc(tokens,
                                         capacity * sizeof(char *));
                        if (!tokens) DIE("realloc failed");
                    }
                    tokens[n++] = strdup(normalized);
                }

                buf_len = 0;
            }

            if (!*p)
                break;
        }
    }

    *count = n;
    return tokens;
}


# Indexer Makefile (GNU Make 3.81)

\
BIN = indexer_c
\
FT_ROOT = ./ft911_decoded
\
STOPS = ./stopwordlist.txt

\
CC = cc
\
CFLAGS = -std=c11 -Wall -Wextra -Wpedantic -O2
\
INCLUDE = -Iinclude

\
SRC = \
\
    src/main.c \
\
    src/strset.c \
\
    src/stopwords.c \
\
    src/tokenizer.c \
\
    src/ft_parser.c \
\
    src/dictionary.c \
\
    src/doc_table.c \
\
    src/forward_index.c \
\
    src/inverted_index.c \
\
    src/indexer.c \
\
    src/query.c
\
OBJ = $(SRC:.c=.o)

\
all: $(BIN)

\
$(BIN): $(OBJ)
	$(CC) $(CFLAGS) $(INCLUDE) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDE) -c $< -o $@

run: all
	./$(BIN) $(FT_ROOT) $(STOPS)

clean:
	rm -f $(OBJ) $(BIN)

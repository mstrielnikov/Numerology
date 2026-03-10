CC = gcc
CFLAGS = -Wall -Wextra -O3 -std=c11 -I include
LDFLAGS = -lm

SRC = src/bigint.c
OBJ = $(SRC:.c=.o)

TEST_SRC = tests/test.c
TEST_BIN = tests/test

.PHONY: all clean test

all: $(OBJ)

$(OBJ): $(SRC) include/bigint.h

test: $(TEST_BIN)
	./$(TEST_BIN)

$(TEST_BIN): $(TEST_SRC) $(OBJ)
	$(CC) $(CFLAGS) $(TEST_SRC) $(OBJ) -o $@ $(LDFLAGS)

clean:
	rm -f $(OBJ) $(TEST_BIN)

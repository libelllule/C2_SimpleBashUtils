CC=gcc
CFLAGS = -Werror -Wextra -Wall -std=c11
all: s21_grep
s21_grep: s21_grep.c s21_grep.h
	$(CC) $(CFLAGS) s21_grep.c -o s21_grep
re: clean all
clean:
	rm -f s21_grep
.PHONY: clean all re


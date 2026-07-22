CC=gcc
CFLAGS = -Werror -Wextra -Wall -std=c11
all: s21_cat
s21_cat: s21_cat.c
	$(CC) $(CFLAGS) s21_cat.c -o s21_cat
re: clean all
clean:
	rm -f s21_cat
.PHONY: clean all re
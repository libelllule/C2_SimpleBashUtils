#ifndef S21_GREP_H
#define S21_GREP_H

#define _GNU_SOURCE
#include <regex.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define FLAGS "e:ivclnhsf:o"

typedef struct {
  bool ignore_reg;
  bool search_inversion;
  bool rows_count;
  bool fnames_only;
  bool row_nums;
  bool no_fnames;
  bool suppress_err;
  bool truncated;
} Flags;

bool add_parametr_from_file(char **pattern, const char *arg);
void copy(char **pattern, const char *arg, int len, int len_arg);
bool add_parametr(char **pattern, const char *arg);
bool parsing(int argc, char *const argv[], Flags *f, char **pattern);
bool find_pattern(FILE *file, const char *filename, const regex_t *regex,
                  const Flags *f, int files_count);
void trancated_string(const char *line, const regex_t *regex, const Flags *f,
                      int count_string, int files_count, const char *filename);
int compile_regex(const Flags *f, regex_t *regex, char *pattern);
void process_files(int argc, char *argv[], int optind, const Flags *f,
                   const regex_t *regex, bool *any_match, int *exit_code);
void print_prefix(const Flags *f, int files_count, const char *filename,
                  int count_string);
void print_summary(const Flags *f, const char *filename, int count_matches,
                   int files_count);
void print_matched_line(const char *line, ssize_t len, const regex_t *regex,
                        const Flags *f, int count_string, int files_count,
                        const char *filename);
bool check_and_set_pattern(int argc, char *const argv[], char **pattern);

#endif
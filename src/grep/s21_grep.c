#include "s21_grep.h"

int main(int argc, char *argv[]) {
  Flags f = {0};
  regex_t regex;
  char *pattern = NULL;
  int exit_code = 0;
  bool any_match = false;
  bool success = parsing(argc, argv, &f, &pattern);
  if (success) {
    if (compile_regex(&f, &regex, pattern) == 0) {
      if (optind == argc) {
        any_match =
            find_pattern(stdin, "(standard input)", &regex, &f, argc - optind);
      } else {
        process_files(argc, argv, optind, &f, &regex, &any_match, &exit_code);
      }
      regfree(&regex);
    } else {
      exit_code = 2;
    }
  } else {
    exit_code = 2;
  }
  if (exit_code == 0 && !any_match) {
    exit_code = 1;
  }
  free(pattern);
  return exit_code;
}

int compile_regex(const Flags *f, regex_t *regex, char *pattern) {
  int comp;
  if (f->ignore_reg) {
    comp = regcomp(regex, pattern, REG_ICASE | REG_NEWLINE);
  } else {
    comp = regcomp(regex, pattern, REG_NEWLINE);
  }
  if (comp != 0) {
    char error_message[100];
    regerror(comp, regex, error_message, sizeof(error_message));
    fprintf(stderr, "grep: %s\n", error_message);
  }
  return comp;
}

void process_files(int argc, char *argv[], int optind, const Flags *f,
                   const regex_t *regex, bool *any_match, int *exit_code) {
  for (int i = optind; i < argc; i++) {
    FILE *file = fopen(argv[i], "r");
    const char *filename = argv[i];
    if (file == NULL) {
      if (f->suppress_err == false) {
        fprintf(stderr, "grep: %s: No such file or directory\n", argv[i]);
      }
      *exit_code = 2;
    } else {
      bool curent_match = find_pattern(file, filename, regex, f, argc - optind);
      if (curent_match) {
        *any_match = true;
      }
      fclose(file);
    }
  }
}
bool find_pattern(FILE *file, const char *filename, const regex_t *regex,
                  const Flags *f, int files_count) {
  char *line = NULL;
  ssize_t len = 0;
  int count_string = 1;
  int count_matches = 0;
  size_t n = 0;
  while ((len = getline(&line, &n, file)) != -1) {
    if (len > 0 && line[len - 1] == '\n') {
      line[len - 1] = '\0';
    }
    bool is_match = false;
    if (regexec(regex, line, 0, NULL, 0) == 0) {
      is_match = true;
    }
    if (f->search_inversion) {
      is_match = !is_match;
    }
    if (is_match) {
      count_matches++;
      if (!f->rows_count && !f->fnames_only) {
        print_matched_line(line, len, regex, f, count_string, files_count,
                           filename);
      }
    }
    count_string++;
  }
  print_summary(f, filename, count_matches, files_count);
  free(line);
  return count_matches > 0;
}

void print_matched_line(const char *line, ssize_t len, const regex_t *regex,
                        const Flags *f, int count_string, int files_count,
                        const char *filename) {
  if (f->truncated) {
    trancated_string(line, regex, f, count_string, files_count, filename);
  } else {
    print_prefix(f, files_count, filename, count_string);
    printf("%s", line);
    if (len > 0 && line[len - 1] != '\n') {
      printf("\n");
    }
  }
}

void print_summary(const Flags *f, const char *filename, int count_matches,
                   int files_count) {
  if (f->rows_count && f->fnames_only) {
    if (count_matches > 0) {
      printf("%s\n", filename);
    } else {
      if (!f->no_fnames && files_count > 1) {
        printf("%s:0\n", filename);
      } else {
        printf("0\n");
      }
    }
    return;
  }

  if (f->rows_count) {
    if (!f->no_fnames && files_count > 1) {
      printf("%s:", filename);
    }
    printf("%d\n", count_matches);
  }

  if (f->fnames_only && count_matches > 0 && !f->rows_count) {
    printf("%s\n", filename);
  }
}

void print_prefix(const Flags *f, int files_count, const char *filename,
                  int count_string) {
  if (files_count > 1 && !f->no_fnames) {
    printf("%s:", filename);
  }
  if (f->row_nums) {
    printf("%d:", count_string);
  }
}
void trancated_string(const char *line, const regex_t *regex, const Flags *f,
                      int count_string, int files_count, const char *filename) {
  size_t nmatch = 1;
  regmatch_t pmatch[1];
  int eflags = 0;

  while ((regexec(regex, line, nmatch, pmatch, eflags)) == 0) {
    int length = pmatch[0].rm_eo - pmatch[0].rm_so;
    if (length == 0) {
      line = line + 1;
    } else {
      print_prefix(f, files_count, filename, count_string);
      printf("%.*s\n", length, line + pmatch[0].rm_so);
      line = line + pmatch[0].rm_eo;
    }
    eflags = REG_NOTBOL;
  }
}

bool parsing(int argc, char *const argv[], Flags *f, char **pattern) {
  bool success = true;
  int opt;
  while ((opt = getopt(argc, argv, FLAGS)) != -1) {
    if (opt == 'i') {
      f->ignore_reg = true;
    } else if (opt == 'v') {
      f->search_inversion = true;
    } else if (opt == 'c') {
      f->rows_count = true;
    } else if (opt == 'l') {
      f->fnames_only = true;
    } else if (opt == 'n') {
      f->row_nums = true;
    } else if (opt == 'h') {
      f->no_fnames = true;
    } else if (opt == 's') {
      f->suppress_err = true;
    } else if (opt == 'o') {
      f->truncated = true;
    } else if (opt == 'e') {
      if (!add_parametr(pattern, optarg)) success = false;
    } else if (opt == 'f') {
      success = add_parametr_from_file(pattern, optarg);
    } else if (opt == '?') {
      if ((optopt == 'f') || (optopt == 'e')) {
        fprintf(stderr, "grep: option requires an argument -- '%c'\n", optopt);
        success = false;
      } else {
        fprintf(stderr, "grep: invalid option -- '%c'\n", optopt);
        success = false;
      }
    }
  }
  if (success) {
    success = check_and_set_pattern(argc, argv, pattern);
  }
  return success;
}

bool check_and_set_pattern(int argc, char *const argv[], char **pattern) {
  bool success = true;
  if (*pattern == NULL) {
    if (optind < argc) {
      if (!add_parametr(pattern, argv[optind])) success = false;
      optind++;
    } else {
      fprintf(stderr, "grep: search pattern not specified\n");
      success = false;
    }
  }
  return success;
}

bool add_parametr(char **pattern, const char *arg) {
  bool success = true;
  bool null_str = false;
  int len = 0, add_byte = 1;
  if (*pattern == NULL) {
    null_str = true;
  } else {
    len = strlen(*pattern);
    add_byte = 3;
  }
  int len_arg = strlen(arg);
  char *tmp = NULL;
  tmp = realloc(*pattern, sizeof(char) * (len + add_byte + len_arg));
  if (tmp == NULL) {
    fprintf(stderr, "grep: memory allocation failed\n");
    success = false;
  } else {
    *pattern = tmp;
    if (null_str) {
      copy(pattern, arg, len, len_arg);
    } else {
      (*pattern)[len] = '\\';
      (*pattern)[len + 1] = '|';
      copy(pattern, arg, len + 2, len_arg);
    }
  }
  return success;
}

void copy(char **pattern, const char *arg, int len, int len_arg) {
  int k = 0;
  for (int i = len; i < (len + len_arg); i++) {
    (*pattern)[i] = arg[k];
    k++;
  }
  (*pattern)[len + len_arg] = '\0';
}

bool add_parametr_from_file(char **pattern, const char *arg) {
  bool success = true;
  FILE *file = fopen(arg, "r");
  if (file == NULL) {
    success = false;
    fprintf(stderr, "grep: %s: No such file or directory\n", arg);
  } else {
    char *line = NULL;
    size_t n = 0;
    ssize_t len = 0;
    while ((len = getline(&line, &n, file)) != -1 && success) {
      if (line[len - 1] == '\n') {
        line[len - 1] = '\0';
      }
      if (!add_parametr(pattern, line)) success = false;
    }
    free(line);
    fclose(file);
  }
  return success;
}

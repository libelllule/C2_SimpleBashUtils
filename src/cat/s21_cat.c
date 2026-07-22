#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define N "--number"
#define B "--number-nonblank"
#define S "--squeeze-blank"

typedef struct {
  bool number;
  bool number_nonblank;
  bool squeeze_blank;
  bool show_ends;
  bool show_nonprinting;
  bool show_tabs;
} flags;

typedef struct {
  int num_line;
  int count_string;
  bool start_string;
} state;

void output_file(FILE *file, flags f, state *s);
void parsing_flags(int argc, char *argv[], int *file_index, bool *success,
                   flags *f);
void operating_file(const char *arg, state *s, bool *success, flags f);
int check_short_flags(const char *arg, flags *f);
int check_long_flags(const char *argv, flags *f);
void check_show_nonprinting(int c);
void check_number(int c, flags f, state *s);
void check_squeeze_blank(int c, flags f, state *s, bool *skip);

int main(int argc, char *argv[]) {
  flags f = {0};
  state s = {.num_line = 1, .count_string = 0, .start_string = true};
  bool success = true;
  int file_index = 1;
  if (argc == 1) {
    output_file(stdin, f, &s);
  } else if (argc > 1) {
    parsing_flags(argc, argv, &file_index, &success, &f);
  }
  if (success) {
    for (int i = file_index; i < argc; i++) {
      operating_file(argv[i], &s, &success, f);
    }
    if (file_index == argc) {
      output_file(stdin, f, &s);
    }
  }
  return success ? 0 : 1;
}

void output_file(FILE *file, flags f, state *s) {
  int c;
  if (f.number_nonblank) {
    f.number = false;
  }
  while ((c = fgetc(file)) != EOF) {
    bool skip = false;
    check_squeeze_blank(c, f, s, &skip);
    if (skip == false) {
      check_number(c, f, s);

      if (f.show_ends && c == '\n') {
        printf("$");
      }
      if (f.show_tabs && c == '\t') {
        printf("^I");
      } else if (f.show_nonprinting) {
        check_show_nonprinting(c);
      } else {
        putchar(c);
      }
      if (c == '\n') {
        s->start_string = true;
      } else {
        s->start_string = false;
      }
    }
  }
}

void check_squeeze_blank(int c, flags f, state *s, bool *skip) {
  if (f.squeeze_blank && s->start_string && c == '\n') {
    (s->count_string)++;
    if (s->count_string > 1) {
      *skip = true;
    }
  } else if (c != '\n') {
    s->count_string = 0;
  }
}

void check_number(int c, flags f, state *s) {
  if (s->start_string) {
    if (f.number || (f.number_nonblank && c != '\n')) {
      printf("%6d\t", s->num_line);
      (s->num_line)++;
    }
  }
}

void check_show_nonprinting(int c) {
  bool is_more = false;
  if (c > 127) {
    putchar('M');
    putchar('-');
    c = c - 128;
    is_more = true;
  }
  if (c == 127) {
    putchar('^');
    putchar('?');
  } else if ((is_more || (c != '\t' && c != '\n')) && c <= 31) {
    putchar('^');
    putchar(c + 64);
  } else {
    putchar(c);
  }
}

int check_long_flags(const char *argv, flags *f) {
  bool valid = false;
  if (strcmp(argv, N) == 0) {
    f->number = true;
    valid = true;
  } else if (strcmp(argv, B) == 0) {
    f->number_nonblank = true;
    valid = true;
  } else if (strcmp(argv, S) == 0) {
    f->squeeze_blank = true;
    valid = true;
  }
  return valid;
}

int check_short_flags(const char *arg, flags *f) {
  bool valid = true;
  for (int i = 1; arg[i] != '\0' && valid; i++) {
    if (arg[i] == 'b') {
      f->number_nonblank = true;
    } else if (arg[i] == 'e') {
      f->show_ends = true;
      f->show_nonprinting = true;
    } else if (arg[i] == 'n') {
      f->number = true;
    } else if (arg[i] == 's') {
      f->squeeze_blank = true;
    } else if (arg[i] == 't') {
      f->show_tabs = true;
      f->show_nonprinting = true;
    } else if (arg[i] == 'T') {
      f->show_tabs = true;
    } else if (arg[i] == 'E') {
      f->show_ends = true;
    } else if (arg[i] == 'v') {
      f->show_nonprinting = true;
    } else {
      valid = false;
    }
  }
  return valid;
}

void operating_file(const char *arg, state *s, bool *success, flags f) {
  FILE *arg_file = fopen(arg, "r");
  if (arg_file == NULL) {
    *success = false;
    fprintf(stderr, "cat: %s: No such file or directory\n", arg);
  } else {
    output_file(arg_file, f, s);
    fclose(arg_file);
  }
}

void parsing_flags(int argc, char *argv[], int *file_index, bool *success,
                   flags *f) {
  bool parsing_flag = true;
  for (int i = 1; i < argc && parsing_flag; i++) {
    if (argv[i][0] == '-' && strlen(argv[i]) > 1) {
      if (argv[i][1] == '-') {
        if (strlen(argv[i]) == 2) {
          parsing_flag = false;
          *file_index = i + 1;
        } else if ((check_long_flags(argv[i], f)) != true) {
          fprintf(stderr, "Неверный флаг %s\n", argv[i]);
          *success = false;
          parsing_flag = false;
        } else {
          *file_index = i + 1;
        }
      } else {
        if (check_short_flags(argv[i], f) != true) {
          fprintf(stderr, "Неверный флаг %s\n", argv[i]);
          *success = false;
          parsing_flag = false;
        } else {
          *file_index = i + 1;
        }
      }
    } else {
      parsing_flag = false;
      *file_index = i;
    }
  }
}

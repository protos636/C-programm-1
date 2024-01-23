#include <ctype.h>
#include <getopt.h>
#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void scan_files(int flags[], int argc, char *argv[]);
void noflag_usage(FILE *fp, int const flags[]);
void printFromFile(FILE *fp, int const *flags);

int main(int argc, char *argv[]) {
  setlocale(LC_CTYPE, "");

  int ch;
  int bflag = 0, eflag = 0, sflag = 0, tflag = 0, vflag = 0, nflag = 0,
      Eflag = 0, Tflag = 0;
  const struct option long_options[] = {{"number-nonblank", 0, NULL, 'b'},
                                        {"number", 0, NULL, 'n'},
                                        {"squeeze-blank", 0, NULL, 's'},
                                        {NULL, 0, NULL, 0}};

  while ((ch = getopt_long(argc, argv, "+benstvET", long_options, NULL)) != -1)
    switch (ch) {
      case 'b':
        bflag = nflag = 1;
        break;
      case 'e':
        eflag = vflag = 1;
        break;
      case 'n':
        nflag = 1;
        break;
      case 's':
        sflag = 1;
        break;
      case 't':
        tflag = vflag = 1;
        break;
      case 'v':
        vflag = 1;
        break;
      case 'E':
        Eflag = 1;
        break;
      case 'T':
        Tflag = 1;
        break;
    }

  int flags[8] = {bflag, eflag, nflag, sflag, tflag, vflag, Eflag, Tflag};
  scan_files(flags, argc, argv);
  return 0;
}

void scan_files(int flags[], int argc, char *argv[]) {
  argc -= optind;
  argv += optind;

  char **filenames = calloc(argc, sizeof(char *));
  // for (int i = 0; i < argc - 1; ++i) filenames[i] = calloc(100,
  // sizeof(char));

  for (int f = 0; f < argc; ++f) {
    FILE *fp = fopen(argv[f], "r");
    if (!fp) {
      printf("cat: %s: No such file or directory\n", argv[f]);
      for (int i = 0; i < argc - 1; ++i) free(filenames[i]);
      free(filenames);
      exit(1);
    } else {
      if (argc > 0) {
        filenames[f] = argv[f];

        if (flags[0] || flags[1] || flags[2] || flags[3] || flags[4] ||
            flags[5])
          printFromFile(fp, flags);
        else
          noflag_usage(fp, flags);
      }
      fclose(fp);
    }
  }
  free(filenames);
}

void printFromFile(FILE *fp, int const *flags) {
  if (fp == stdin && feof(stdin)) clearerr(stdin);

  if (flags[2]) {
    int ch;
    int countLines = 0;
    for (int prev = '\n'; (ch = getc(fp)) != EOF; prev = ch) {
      if ((!flags[0] || ch != '\n')) {
        if (prev == '\n') {
          (void)fprintf(stdout, "%6d\t", ++countLines);
        }
      }
      putchar(ch);
    }
  }

  else if (flags[0]) {
    int ch;
    int countLines = 0;
    for (int prev = '\n'; (ch = getc(fp)) != EOF; prev = ch) {
      if (prev == '\n' && ch > 31 && ch < 128) {
        (void)fprintf(stdout, "%6d\t", ++countLines);
      }
      putchar(ch);
    }
    putchar('\n');
  }

  else if (flags[3]) {
    int gobble = 0;
    int ch, prev;
    for (prev = '\n'; (ch = getc(fp)) != EOF; prev = ch) {
      if (prev == '\n') {
        if (ch == '\n') {
          if (gobble) continue;
          gobble = 1;
        } else
          gobble = 0;
      }
      if (putchar(ch) == EOF) break;
    }
  }

  else if (flags[5]) {
    int ch;

    while ((ch = getc(fp)) != EOF) {
      if ((ch <= '\37' && ch != '\n' && ch != '\t')) {
        printf("^%c", ch ^ 0b01000000);
      }
      if (ch == 9 && flags[4]) {
        printf("^I");
      }

      if (flags[1] && (ch == 10))
        printf("$%c", ch);
      else if (ch == 10)
        printf("%c", ch);
      if ((ch >= 32 && ch != 127) || (ch == 9 && !flags[4])) printf("%c", ch);
      if (ch == 127) printf("^?");
    }
  }
}

void noflag_usage(FILE *fp, int const flags[]) {
  int ch;

  while ((ch = getc(fp)) != EOF) {
    if (flags[6] == 1 && (ch == 10)) putchar('$');

    if (flags[7] && ch == '\t') {
      printf("^");
      ch = '\t' + 64;
    }
    putchar(ch);
  }
}

#include <ctype.h>
#include <errno.h>
#include <getopt.h>
#include <locale.h>
#include <regex.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

void input_comp(int argc, char *argv[], int flags[]);
void cook_grep(FILE *fp, int const flags[], char const *filenames,
               regex_t const *prx, int countFile);
void grep_count(FILE *fp, char const *filenames, int const flags[],
                regex_t const *prx, int countFile);
void add_pattern(int argc, char *argv[], int flags[], char epat[], char fpat[]);
void flag_o(char *line, regex_t const *prx, regmatch_t *match, int countFile,
            char const *filenames);
void read_patterns(char const *fn, int argc, char *argv[]);
void f_flag(int argc, char *argv[], regex_t *pfrx, int flags[]);

int main(int argc, char *argv[]) {
  int eflag = 0, iflag = 0, vflag = 0, cflag = 0, lflag = 0, nflag = 0,
      cflags = 0, hflag = 0, sflag = 0, oflag = 0, fflag = 0;

  int ch = 0;
  char epat[10000] = {0};
  while ((ch = getopt(argc, argv, "+e:ivclnhsof:")) != -1) switch (ch) {
      case 'e':
        eflag = 1;
        cflags |= REG_EXTENDED;
        strcat(epat, optarg);
        strcat(epat, "|");
        break;
      case 'i':
        iflag = 1;
        cflags |= REG_ICASE;
        break;
      case 'v':
        vflag = 1;
        break;
      case 'c':
        cflag = 1;
        break;
      case 'l':
        lflag = 1;
        break;
      case 'n':
        nflag = 1;
        break;
      case 'h':
        hflag = 1;
        break;
      case 's':
        sflag = 1;
        break;
      case 'o':
        oflag = 1;
        break;
      case 'f':
        fflag = 1;
        read_patterns(optarg, argc, argv);
        break;
    }
  int flags[11] = {eflag,  iflag, vflag, cflag, lflag, nflag,
                   cflags, hflag, sflag, oflag, fflag};
  if (eflag)
    add_pattern(argc, argv, flags, epat, 0);
  else if (fflag)
    ;
  else
    input_comp(argc, argv, flags);
  return 0;
}

void read_patterns(char const *fn, int argc, char *argv[]) {
  char pattern[8000] = {0};
  FILE *f;
  if ((f = fopen(fn, "r")) == NULL) {
    perror(fn);
    exit(1);
  } else {
    int fflags[11] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1};
    char fpat[10000] = {0};
    while (!feof(f)) {
      if ((fgets(pattern, sizeof(pattern), f)) != NULL) {
        if (pattern[strlen(pattern) - 1] == '\n' && strlen(pattern) - 1 != 0)
          pattern[strlen(pattern) - 1] = '\0';
        strcat(fpat, pattern);
        strcat(fpat, "|");
      }
    }
    fclose(f);
    add_pattern(argc, argv, fflags, 0, fpat);
  }
}

void add_pattern(int argc, char *argv[], int flags[], char epat[],
                 char fpat[]) {
  regex_t rx;
  if (flags[0]) {
    epat[strlen(epat) - 1] = '\0';
    if ((regcomp(&rx, epat, REG_EXTENDED))) printf("Не скопилилил");
  }
  if (flags[10]) {
    fpat[strlen(fpat) - 1] = '\0';
    if ((regcomp(&rx, fpat, REG_EXTENDED))) printf("Не скопилилил");
  }
  argc -= optind;
  argv += optind;
  char **filenames = calloc(argc, sizeof(char *));
  if (filenames == NULL) printf("Память не выделалась");
  for (int f = 0; f < argc; ++f) {
    FILE *fp = fopen(argv[f], "r");
    if (!fp) {
      if (flags[8] == 0) {
        printf("s21_grep: %s: No such file or directory\n", argv[f]);
      }
    } else {
      if (argc > 0) {
        filenames[f] = argv[f];
        cook_grep(fp, flags, filenames[f], &rx, argc);
        fclose(fp);
      }
    }
  }
  free(filenames);
  regfree(&rx);
}

void grep_count(FILE *fp, char const *filenames, int const flags[],
                regex_t const *prx, int countFile) {
  char line[8000] = {0};
  regmatch_t matches;
  int countLines = 0, rawLines = 0;
  while ((fgets(line, sizeof(line), fp)) != NULL) {
    ++rawLines;
    if (!(regexec(prx, line, 1, &matches, 0))) {
      ++countLines;
      if (flags[5] == 1) {
        if (countFile == 1)
          printf("%d:%s", rawLines, line);
        else
          printf("%s:%d:%s", filenames, rawLines, line);
      }
    }
  }
  if (countFile == 1) {
    if (flags[3] == 1) printf("%d\n", countLines);
    if (flags[4] == 1 && countLines > 0) printf("%s\n", filenames);
  } else {
    if (flags[3] == 1) printf("%s:%d\n", filenames, countLines);
    if (flags[4] == 1 && countLines > 0) printf("%s\n", filenames);
  }
}

void flag_o(char *line, regex_t const *prx, regmatch_t *match, int countFile,
            char const *filenames) {
  char *pointer = line;
  if (countFile == 1) {
    while (!(regexec(prx, pointer, 1, match, 0))) {
      printf("%.*s\n", (int)(match->rm_eo - match->rm_so),
             pointer + match->rm_so);
      pointer += match[0].rm_eo;
    }
  } else {
    while (!(regexec(prx, pointer, 1, match, 0))) {
      printf("%s:%.*s\n", filenames, (int)(match->rm_eo - match->rm_so),
             pointer + match->rm_so);
      pointer += match[0].rm_eo;
    }
  }
}

void cook_grep(FILE *fp, int const flags[], char const *filenames,
               regex_t const *prx, int countFile) {
  char line[8000] = {0};
  regmatch_t matches;
  regmatch_t *match = &matches;

  while ((fgets(line, sizeof(line), fp)) != NULL) {
    if (countFile == 1) {
      if (flags[0] == 0 && flags[1] == 0 && flags[2] == 0 && flags[3] == 0 &&
          flags[4] == 0 && flags[5] == 0 && flags[7] == 0 && flags[8] == 0 &&
          flags[9] == 0 && flags[10] == 0) {
        if (!(regexec(prx, line, 1, &matches, 0))) printf("%s", line);
      } else if (flags[1] == 1) {
        if (!(regexec(prx, line, 1, &matches, 0))) printf("%s", line);
      } else if (flags[0] == 1) {
        if (!(regexec(prx, line, 1, &matches, 0))) printf("%s", line);
      } else if (flags[2] == 1) {
        if ((regexec(prx, line, 1, &matches, 0))) printf("%s", line);
      } else if (flags[7] == 1) {
        if (!(regexec(prx, line, 1, &matches, 0))) printf("%s", line);
      } else if (flags[8] == 1) {
        if (!(regexec(prx, line, 1, &matches, 0))) printf("%s", line);
      } else if (flags[9]) {
        if (!(regexec(prx, line, 1, &matches, 0)))
          flag_o(line, prx, match, countFile, filenames);
      } else if (flags[10]) {
        if (!(regexec(prx, line, 1, &matches, 0))) printf("%s", line);
      }
    } else {
      if (flags[0] == 0 && flags[1] == 0 && flags[2] == 0 && flags[3] == 0 &&
          flags[4] == 0 && flags[5] == 0 && flags[6] == 0 && flags[7] == 0 &&
          flags[8] == 0 && flags[9] == 0 && flags[10] == 0) {
        if (!(regexec(prx, line, 1, &matches, 0)))
          printf("%s:%s", filenames, line);
      } else if (flags[1]) {
        if (!(regexec(prx, line, 1, &matches, 0)))
          printf("%s:%s", filenames, line);
      } else if (flags[0]) {
        if (!(regexec(prx, line, 1, &matches, 0)))
          printf("%s:%s", filenames, line);
      } else if (flags[2]) {
        if ((regexec(prx, line, 1, &matches, 0)))
          printf("%s:%s", filenames, line);
      } else if (flags[7]) {
        if (!(regexec(prx, line, 1, &matches, 0))) printf("%s", line);
      } else if (flags[9]) {
        if (!(regexec(prx, line, 1, &matches, 0)))
          flag_o(line, prx, match, countFile, filenames);
      } else if (flags[8] == 1) {
        if (!(regexec(prx, line, 1, &matches, 0)))
          printf("%s:%s", filenames, line);
      } else if (flags[10]) {
        if (!(regexec(prx, line, 1, &matches, 0)))
          printf("%s:%s", filenames, line);
      }
    }
  }
}

void input_comp(int argc, char *argv[], int flags[]) {
  char **end = &argv[argc];
  char **pattern = &argv[1];
  regex_t rx;
  for (; pattern != end && pattern[0][0] == '-'; ++pattern)
    ;
  if (pattern == end) fprintf(stderr, "no pattern\n");
  if ((regcomp(&rx, *pattern, flags[6]))) printf("Не скопилилил");
  int countFile = 0;
  for (char **filenames = pattern + 1; filenames != end; ++filenames) {
    if (**filenames == '-') continue;
    ++countFile;
    if (countFile >= 2) break;
  }
  for (char **filenames = pattern + 1; filenames != end; ++filenames) {
    if (**filenames == '-') continue;
    FILE *fp = fopen(*filenames, "r");
    if (!fp) {
      if (!flags[8]) printf("no such file\n");
    } else {
      if (flags[3] || flags[4] || flags[5])
        grep_count(fp, *filenames, flags, &rx, countFile);
      else
        cook_grep(fp, flags, *filenames, &rx, countFile);
      fclose(fp);
    }
  }
  regfree(&rx);
}

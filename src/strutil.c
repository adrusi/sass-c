#include "strutil.h"

char *str_next_line(char *str) {
  for (; *str != '\n'; str++);
  return str;
}

int str_line_len(char *str) {
  int len;
  for (len = 0; str[len] != '\n'; len++);
  return len;
}

int str_has_prefix(char *str, char *pre) {
  int i;
  for (i = 0; pre[i] && str[i]; i++) if (pre[i] != str[i]) return 0;
  return 1;
}

int str_index_of_first_occurance(char *str, char *pat, int limit) {
  if (limit < 0) limit = ((unsigned int) -1) >> 1; // max_int
  int i;
  for (i = 0; str[i] && i < limit; i++)
  if (str_has_prefix(str + i, pat)) {
    return i;
  }
  return -1;
}




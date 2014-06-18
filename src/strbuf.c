#include "strbuf.h"
#include <stdlib.h>
#include <string.h>

#define INITIAL_CAPACITY 128
strbuf strbuf_new(void) {
  char *str = malloc(INITIAL_CAPACITY * sizeof (char));
  str[0] = '\0';
  return (strbuf) {
    .string = str,
    .len = 0,
    .cap = INITIAL_CAPACITY
  };
}

void strbuf_append(strbuf *buf, char *str) {
  int len = strlen(str);
  int endlen = len + buf->len;
  int oldcap = buf->cap;
  while (endlen >= buf->cap) buf->cap *= 2;
  if (buf->cap != oldcap) buf->string = realloc(buf->string, buf->cap);
  int i;
  for (i = 0; str[i]; i++) buf->string[buf->len++] = str[i];
  buf->string[buf->len] = '\0';
}

void strbuf_appendchar(strbuf *buf, char c) {
  if (buf->len + 1 >= buf->cap) {
    buf->cap *= 2;
    buf->string = realloc(buf->string, buf->cap);
  }
  buf->string[buf->len++] = c;
  buf->string[buf->len] = '\0';
}

void strbuf_free(strbuf buf) {
  free(buf.string);
}

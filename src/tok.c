#include "tok.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "strbuf.h"
#include "strutil.h"

char *tok_mnemonics[64];
char *tok_registers[64];

static int alphabetic(char c) {
  return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}
static int numeric(char c) {
  return (c >= '0' && c <= '9');
}

int tokread_eof(FILE *f, tok *t) {
  t->type = TOK_EOF;
  long mark = ftell(f);
  char c = fgetc(f);
  fseek(f, mark, SEEK_SET);
  return c == EOF;
}

int tokread_sym(FILE *f, tok *t) {
  t->type = TOK_SYM;
  char c = fgetc(f);
  if (!(c == '_' || alphabetic(c))) return 0;
  strbuf s = strbuf_new();
  strbuf_appendchar(&s, c);
  c = fgetc(f);
  while (c == '_' || alphabetic(c) || numeric(c)) {
    strbuf_appendchar(&s, c);
    c = fgetc(f);
  }
  if (c != EOF) fseek(f, -1, SEEK_CUR);
  t->data.sym = realloc(s.string, s.len);
  return 1;
}

int tokread_ins(FILE *f, tok *t) {
  t->type = TOK_INS;
  strbuf s = strbuf_new();
  char c;
  while (alphabetic(c = fgetc(f))) strbuf_appendchar(&s, c);
  if (c != EOF) fseek(f, -1, SEEK_CUR);
  int i;
  for (i = 0; tok_mnemonics[i]; i++)
  if (strcmp(tok_mnemonics[i], s.string) == 0) {
    t->data.ins = i;
    strbuf_free(s);
    return 1;
  }
  strbuf_free(s);
  return 0;
}

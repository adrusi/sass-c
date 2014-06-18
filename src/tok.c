#include "tok.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "strbuf.h"
#include "strutil.h"
#include "dbg.h"

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

int tokread_reg(FILE *f, tok *t) {
  t->type = TOK_REG;
  strbuf s = strbuf_new();
  char c;
  while (alphabetic(c = fgetc(f))) strbuf_appendchar(&s, c);
  if (c != EOF) fseek(f, -1, SEEK_CUR);
  int i;
  for (i = 0; tok_registers[i]; i++)
  if (strcmp(tok_registers[i], s.string) == 0) {
    t->data.reg = i;
    strbuf_free(s);
    return 1;
  }
  strbuf_free(s);
  return 0;
}

static int read_dec(FILE *f, tok *t);
static int read_hex(FILE *f, tok *t);
static int read_oct(FILE *f, tok *t);
static int read_bin(FILE *f, tok *t);
static int read_char(FILE *f, tok *t);
static int read_bool(FILE *f, tok *t);

#define NUM_NUM_READERS 6
int tokread_num(FILE *f, tok *t) {
  t->type = TOK_NUM;
  static tok_reader readers[NUM_NUM_READERS] = {
    read_hex, read_oct, read_bin, read_dec, read_char, read_bool
  };
  long mark = ftell(f);
  int i;
  for (i = 0; i < NUM_NUM_READERS; i++) {
    if (readers[i](f, t)) return 1;
    else fseek(f, mark, SEEK_SET);
  }
  return 0;
}

static int read_dec(FILE *f, tok *t) {
  t->data.num = 0;
  char c = fgetc(f);
  if (!numeric(c)) return 0;
  do {
    t->data.num *= 10;
    t->data.num += c - '0';
  } while (numeric(c = fgetc(f)));
  if (c != EOF) fseek(f, -1, SEEK_CUR);
  return 1;
}

static int read_hex_c(FILE *f, tok *t);
static int read_hex_asm(FILE *f, tok *t);
static int read_hex_post(FILE *f, tok *t);

#define NUM_HEX_READERS 3
static int read_hex(FILE *f, tok *t) {
  static tok_reader readers[NUM_HEX_READERS] = {
    read_hex_c, read_hex_asm, read_hex_post
  };
  long mark = ftell(f);
  int i;
  for (i = 0; i < NUM_HEX_READERS; i++) {
    if (readers[i](f, t)) return 1;
    else fseek(f, mark, SEEK_SET);
  }
  return 0;
}

static int is_hex_digit(char c) {
  return numeric(c) || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f');
}

static int read_hex_c(FILE *f, tok *t) {
  t->data.num = 0;
  char c;
  c = fgetc(f);
  if (c != '0') return 0;
  c = fgetc(f);
  if (c != 'x') return 0;
  while (is_hex_digit(c = fgetc(f))) {
    t->data.num *= 0x10;
    if (c >= '0' && c <= '9') t->data.num += c - '0';
    else if (c >= 'A' && c <= 'F') t->data.num += c - 'A' + 0xA;
    else t->data.num += c - 'a' + 0xa;
  }
  if (c != EOF) fseek(f, -1, SEEK_CUR);
  return 1;
}

static int read_hex_asm(FILE *f, tok *t) {
  t->data.num = 0;
  char c;
  c = fgetc(f);
  if (c != '$') return 0;
  while (is_hex_digit(c = fgetc(f))) {
    t->data.num *= 0x10;
    if      (c >= '0' && c <= '9') t->data.num += c - '0';
    else if (c >= 'A' && c <= 'F') t->data.num += c - 'A' + 0xA;
    else                           t->data.num += c - 'a' + 0xa;
  }
  if (c != EOF) fseek(f, -1, SEEK_CUR);
  return 1;
}

static int read_hex_post(FILE *f, tok *t) {
  t->data.num = 0;
  long mark = ftell(f);
  char c = fgetc(f);
  if (!is_hex_digit(c)) return 0;
  do {
    t->data.num *= 0x10;
    if      (c >= '0' && c <= '9') t->data.num += c - '0';
    else if (c >= 'A' && c <= 'F') t->data.num += c - 'A' + 0xA;
    else                           t->data.num += c - 'a' + 0xa;
  } while (is_hex_digit(c = fgetc(f)));
  return c == 'h';
}

static int read_oct(FILE *f, tok *t) {
  return 0;
}

static int read_bin(FILE *f, tok *t) {
  return 0;
}

static int read_char(FILE *f, tok *t) {
  return 0;
}

static int read_bool(FILE *f, tok *t) {
  return 0;
}

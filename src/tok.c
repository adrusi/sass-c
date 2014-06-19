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
  if (!(c == '_' || c == '.' || alphabetic(c))) return 0;
  strbuf s = strbuf_new();
  strbuf_appendchar(&s, c);
  c = fgetc(f);
  while (c == '_' || c == '.' || alphabetic(c) || numeric(c)) {
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
  if (!is_hex_digit(c = fgetc(f))) return 0;
  do {
    t->data.num *= 0x10;
    if      (c >= '0' && c <= '9') t->data.num += c - '0';
    else if (c >= 'A' && c <= 'F') t->data.num += c - 'A' + 0xA;
    else                           t->data.num += c - 'a' + 0xa;
  } while (is_hex_digit(c = fgetc(f)));
  if (c != EOF) fseek(f, -1, SEEK_CUR);
  return 1;
}

static int read_hex_post(FILE *f, tok *t) {
  t->data.num = 0;
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

static int read_oct_c(FILE *f, tok *t);
static int read_oct_post(FILE *f, tok *t);

#define NUM_OCT_READERS 2
static int read_oct(FILE *f, tok *t) {
  static tok_reader readers[NUM_OCT_READERS] = {
    read_oct_c, read_oct_post
  };
  long mark = ftell(f);
  int i;
  for (i = 0; i < NUM_OCT_READERS; i++) {
    if (readers[i](f, t)) return 1;
    else fseek(f, mark, SEEK_SET);
  }
  return 0;
}

static int is_octal_digit(char c) {
  return (c >= '0' && c <= '7');
}

static int read_oct_c(FILE *f, tok *t) {
  t->data.num = 0;
  char c;
  c = fgetc(f);
  if (c != '0') return 0;
  if (!is_octal_digit(c = fgetc(f))) return 0;
  do {
    t->data.num *= 010;
    t->data.num += c - '0';
  } while (is_octal_digit(c = fgetc(f)));
  if (c != EOF) fseek(f, -1, SEEK_CUR);
  return 1;
}

static int read_oct_post(FILE *f, tok *t) {
  t->data.num = 0;
  char c = fgetc(f);
  if (!is_octal_digit(c)) return 0;
  do {
    t->data.num *= 010;
    t->data.num += c - '0';
  } while (is_octal_digit(c = fgetc(f)));
  return c == 'o';
}

static int read_bin_c(FILE *f, tok *t);
static int read_bin_asm(FILE *f, tok *t);
static int read_bin_post(FILE *f, tok *t);

#define NUM_BIN_READERS 3
static int read_bin(FILE *f, tok *t) {
  static tok_reader readers[NUM_BIN_READERS] = {
    read_bin_c, read_bin_asm, read_bin_post
  };
  long mark = ftell(f);
  int i;
  for (i = 0; i < NUM_BIN_READERS; i++) {
    if (readers[i](f, t)) return 1;
    else fseek(f, mark, SEEK_SET);
  }
  return 0;
}

static int is_binary_digit(char c) {
  return c == '0' || c == '1';
}

static int read_bin_c(FILE *f, tok *t) {
  t->data.num = 0;
  char c;
  c = fgetc(f);
  if (c != '0') return 0;
  c = fgetc(f);
  if (c != 'b') return 0;
  while (is_binary_digit(c = fgetc(f))) {
    t->data.num *= 2;
    t->data.num += c - '0';
  }
  if (c != EOF) fseek(f, -1, SEEK_CUR);
  return 1;
}

static int read_bin_asm(FILE *f, tok *t) {
  t->data.num = 0;
  char c;
  c = fgetc(f);
  if (c != '%') return 0;
  if (!is_binary_digit(c = fgetc(f))) return 0;
  do {
    t->data.num *= 2;
    t->data.num += c - '0';
  } while (is_binary_digit(c = fgetc(f)));
  if (c != EOF) fseek(f, -1, SEEK_CUR);
  return 1;
}

static int read_bin_post(FILE *f, tok *t) {
  t->data.num = 0;
  char c = fgetc(f);
  if (!is_binary_digit(c)) return 0;
  do {
    t->data.num *= 2;
    t->data.num += c - '0';
  } while (is_binary_digit(c = fgetc(f)));
  return c == 'b';
}

static int read_char(FILE *f, tok *t) {
  t->data.num = 0;
  char c = fgetc(f);
  if (c != '\'') return 0;
  c = fgetc(f);
  if (c != '\\') {
    t->data.num = c;
  } else {
    switch (c = fgetc(f)) {
      case '\'': case '\\':
        t->data.num = c;
        break;
      case 'a':
        t->data.num = '\a';
        break;
      case 'b':
        t->data.num = '\b';
        break;
      case 'f':
        t->data.num = '\f';
        break;
      case 'n':
        t->data.num = '\n';
        break;
      case 'r':
        t->data.num = '\r';
        break;
      case 't':
        t->data.num = '\t';
        break;
      case 'v':
        t->data.num = '\t';
        break;
      case '0': case '1': case '2': case '3':
      case '4': case '5': case '6': case '7':
        do {
          t->data.num *= 010;
          t->data.num += c - '0';
        } while (is_octal_digit(c = fgetc(f)));
        fseek(f, -1, SEEK_CUR);
        break;
      case 'x':
        while (is_hex_digit(c = fgetc(f))) {
          t->data.num *= 0x10;
          if      (c >= '0' && c <= '9') t->data.num += c - '0';
          else if (c >= 'A' && c <= 'F') t->data.num += c - 'A' + 0xA;
          else                           t->data.num += c - 'a' + 0xa;
        }
        fseek(f, -1, SEEK_CUR);
        break;
      default:
        return 0;
    }
  }
  return fgetc(f) == '\'';
}

static int read_bool(FILE *f, tok *t) {
  int res = tokread_sym(f, t);
  t->type = TOK_NUM;
  if (!res) return 0;
  if (strcmp(t->data.sym, "true") == 0) {
    tok_free(*t);
    t->data.num = 1;
    return 1;
  } else if (strcmp(t->data.sym, "false") == 0) {
    tok_free(*t);
    t->data.num = 0;
    return 1;
  } else {
    tok_free(*t);
    return 0;
  }
}

int tokread_brk(FILE *f, tok *t) {
  t->type = TOK_BRK;
  char c = fgetc(f);
  return c == '\\' || c == '\n';
}

static int read_lbl_pre(FILE *f, tok *t);
static int read_lbl_post(FILE *f, tok *t);

#define NUM_LBL_READERS 2
int tokread_lbl(FILE *f, tok *t) {
  t->type = TOK_LBL;
  static tok_reader readers[NUM_LBL_READERS] = {
    read_lbl_pre, read_lbl_post
  };
  long mark = ftell(f);
  int i;
  for (i = 0; i < NUM_LBL_READERS; i++) {
    if (readers[i](f, t)) return 1;
    else fseek(f, mark, SEEK_SET);
  }
  return 0;
}

static int read_lbl_pre(FILE *f, tok *t) {
  if (fgetc(f) != ':') return 0;
  if (!tokread_sym(f, t)) {
    t->type = TOK_LBL;
    tok_free(*t);
    return 0;
  }
  t->type = TOK_LBL;
  return 1;
}

static int read_lbl_post(FILE *f, tok *t) {
  if (!tokread_sym(f, t)) {
    t->type = TOK_LBL;
    tok_free(*t);
    return 0;
  }
  if (fgetc(f) != ':') {
    tok_free(*t);
    return 0;
  }
  t->type = TOK_LBL;
  return 1;
}

#define NUM_OPERATORS 17
int tokread_opr(FILE *f, tok *t) {
  t->type = TOK_OPR;
  static struct { tokdat_opr id; char *str; } ops[NUM_OPERATORS] = {
    {TOKOPR_ADD, "+"},  {TOKOPR_SUB, "-"},  {TOKOPR_MUL, "*"},
    {TOKOPR_DIV, "/"},  {TOKOPR_MOD, "%"},  {TOKOPR_SHR, ">>"},
    {TOKOPR_SHL, "<<"}, {TOKOPR_LTE, "<="}, {TOKOPR_GTE, ">="},
    {TOKOPR_LT,  "<"},  {TOKOPR_GT,  ">"},  {TOKOPR_EQ,  "=="},
    {TOKOPR_NEQ, "!="}, {TOKOPR_AND, "&&"}, {TOKOPR_OR,  "||"},
    {TOKOPR_BND, "&"},  {TOKOPR_BOR, "|"}
  };
  long mark = ftell(f);
  int i;
  char buffer[3];
  for (i = 0; i < NUM_OPERATORS; i++) {
    int len = strlen(ops[i].str);
    fgets(buffer, len + 1, f);
    if (strcmp(buffer, ops[i].str) == 0) {
      t->data.opr = ops[i].id;
      return 1;
    }
    fseek(f, mark, SEEK_SET);
  }
  return 0;
}

int tokread_sep(FILE *f, tok *t) {
  t->type = TOK_SEP;
  return fgetc(f) == ',';
}

int tokread_str(FILE *f, tok *t) {
  t->type = TOK_STR;
  if (fgetc(f) != '"') return 0;
  char c, _c;
  strbuf s = strbuf_new();
  while ((c = fgetc(f)) != '"') {
    if (c != '\\') {
      strbuf_appendchar(&s, c);
      continue;
    }
    switch (c = fgetc(f)) {
      case '"': case '\\':
        strbuf_appendchar(&s, c);
        break;
      case 'a':
        strbuf_appendchar(&s, '\a');
        break;
      case 'b':
        strbuf_appendchar(&s, '\b');
        break;
      case 'f':
        strbuf_appendchar(&s, '\f');
        break;
      case 'n':
        strbuf_appendchar(&s, '\n');
        break;
      case 'r':
        strbuf_appendchar(&s, '\r');
        break;
      case 't':
        strbuf_appendchar(&s, '\t');
        break;
      case 'v':
        strbuf_appendchar(&s, '\v');
        break;
      case '0': case '1': case '2': case '3':
      case '4': case '5': case '6': case '7':
        _c = '\0';
        do {
          _c *= 010;
          _c += c - '0';
        } while (is_octal_digit(c = fgetc(f)));
        fseek(f, -1, SEEK_CUR);
        strbuf_appendchar(&s, _c);
        break;
      case 'x':
        _c = '\0';
        while (is_hex_digit(c = fgetc(f))) {
          _c *= 0x10;
          if      (c >= '0' && c <= '9') _c += c - '0';
          else if (c >= 'A' && c <= 'F') _c += c - 'A' + 0xA;
          else                           _c += c - 'a' + 0xa;
        }
        fseek(f, -1, SEEK_CUR);
        strbuf_appendchar(&s, _c);
        break;
      case 'u':
        // TODO unicode escape codes
        log_err("Unicode escape codes in strings not yet supported.");
        break;
      default:
        strbuf_free(s);
        return 0;
    }
  }
  t->data.str = s.string;
  return 1;
}

#define NUM_DIRECTIVES 17
int tokread_dir(FILE *f, tok *t) {
  t->type = TOK_DIR;
  static struct { tokdat_dir id; char *str; } dirs[NUM_DIRECTIVES] = {
    {TOKDIR_ASCII, "ascii"}, {TOKDIR_ASCIIZ, "asciiz"}, {TOKDIR_ASCIIP, "asciip"},
    {TOKDIR_DB, "db"}, {TOKDIR_DW, "dw"}, {TOKDIR_DEFINE, "define"},
    {TOKDIR_ECHO, "echo"}, {TOKDIR_ELSE, "else"}, {TOKDIR_ENDIF, "endif"},
    {TOKDIR_EQU, "equ"}, {TOKDIR_FILL, "fill"}, {TOKDIR_IFDEF, "ifdef"},
    {TOKDIR_IF, "if"}, {TOKDIR_INCLUDE, "include"}, {TOKDIR_LIST, "list"},
    {TOKDIR_NOLIST, "nolist"}, {TOKDIR_ORG, "org"}
  };
  char c = fgetc(f);
  if (c != '.' && c != '#') return 0;
  char buffer[8];
  long mark = ftell(f);
  int i;
  for (i = 0; i < NUM_DIRECTIVES; i++) {
    int len = strlen(dirs[i].str);
    fgets(buffer, len + 1, f);
    if (strcmp(buffer, dirs[i].str) == 0) {
      t->data.dir = dirs[i].id;
      return 1;
    }
    fseek(f, mark, SEEK_SET);
  }
  return 0;
}

int tokread_beg(FILE *f, tok *t) {
  t->type = TOK_BEG;
  return fgetc(f) == '(';
}

int tokread_end(FILE *f, tok *t) {
  t->type = TOK_END;
  return fgetc(f) == ')';
}

void tok_free(tok t) {
  switch (t.type) {
    case TOK_SYM:
      free(t.data.sym);
      break;
    case TOK_LBL:
      free(t.data.lbl);
      break;
    case TOK_STR:
      free(t.data.str);
      break;
    default:
      break;
  }
}

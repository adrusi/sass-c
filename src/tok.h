#ifndef TOK_H_
#define TOK_H_

#include <stdio.h>

typedef enum {
  TOK_EOF, // end of file
  TOK_SYM, // label usage, macro name
  TOK_INS, // instruction mnemonic
  TOK_REG, // register
  TOK_NUM, // numeric literal
  TOK_BRK, // newline or backslash
  TOK_LBL, // label declaration
  TOK_OPR, // operator (+ * & << etc)
  TOK_SEP, // comma
  TOK_STR, // string literal (for .db directives)
  TOK_DIR, // preprocessor directive
  TOK_BEG, // '('
  TOK_END  // ')'
} tok_type;

typedef char tokdat_eof;
typedef char *tokdat_sym;
typedef int tokdat_ins;
typedef int tokdat_reg;
typedef int tokdat_num;
typedef char tokdat_brk;
typedef char *tokdat_lbl;
typedef enum {
  TOKOPR_ADD, TOKOPR_SUB, TOKOPR_MUL, TOKOPR_DIV, TOKOPR_MOD,
  TOKOPR_SHR, TOKOPR_SHL,
  TOKOPR_LT,  TOKOPR_GT,  TOKOPR_LTE, TOKOPR_GTE,
  TOKOPR_EQ,  TOKOPR_NEQ,
  TOKOPR_BND, TOKOPR_BOR,
  TOKOPR_AND, TOKOPR_OR
} tokdat_opr;
typedef char tokdat_sep;
typedef char *tokdat_str;
typedef enum {
  TOKDIR_ASCII, TOKDIR_ASCIIZ, TOKDIR_ASCIIP, TOKDIR_DB, TOKDIR_DW,
  TOKDIR_DEFINE, TOKDIR_ECHO, TOKDIR_ELSE, TOKDIR_ENDIF, TOKDIR_EQU,
  TOKDIR_FILL, TOKDIR_IF, TOKDIR_IFDEF, TOKDIR_INCLUDE, TOKDIR_LIST,
  TOKDIR_NOLIST, TOKDIR_ORG
} tokdat_dir;
typedef char tokdat_beg;
typedef char tokdat_end;

typedef struct {
  tok_type type;
  int line;
  int col;
  union {
    tokdat_eof eof;
    tokdat_sym sym;
    tokdat_ins ins;
    tokdat_reg reg;
    tokdat_num num;
    tokdat_brk brk;
    tokdat_lbl lbl;
    tokdat_opr opr;
    tokdat_sep sep;
    tokdat_str str;
    tokdat_dir dir;
    tokdat_beg beg;
    tokdat_end end;
  } data;
} tok;

extern char *tok_mnemonics[64];
extern char *tok_registers[64];

typedef int (*tok_reader)(FILE *f, tok *t);

extern int tokread_eof(FILE *f, tok *t);
extern int tokread_sym(FILE *f, tok *t);
extern int tokread_ins(FILE *f, tok *t);
extern int tokread_reg(FILE *f, tok *t);
extern int tokread_num(FILE *f, tok *t);
extern int tokread_brk(FILE *f, tok *t);
extern int tokread_lbl(FILE *f, tok *t);
extern int tokread_opr(FILE *f, tok *t);
extern int tokread_sep(FILE *f, tok *t);
extern int tokread_str(FILE *f, tok *t);
extern int tokread_dir(FILE *f, tok *t);
extern int tokread_beg(FILE *f, tok *t);
extern int tokread_end(FILE *f, tok *t);

extern void tok_free(tok t);
    
#endif

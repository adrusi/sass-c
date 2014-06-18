#include "tok.c"
#include "test.h"
#include <stdio.h>
#include <string.h>

static FILE *file_with_contents(char *str) {
  FILE *f = tmpfile();
  fputs(str, f);
  rewind(f);
  return f;
}

test (tokread_eof) {
  tok t;
  FILE *f;
  f = file_with_contents("");
  assert(tokread_eof(f, &t));
  f = file_with_contents("c");
  assert(!tokread_eof(f, &t));
}

test (tokread_sym) {
  tok t;
  FILE *f;
  f = file_with_contents("abc");
  assert(tokread_sym(f, &t));
  assert(strcmp(t.data.sym, "abc") == 0); 
  f = file_with_contents("_abc");
  assert(tokread_sym(f, &t));
  assert(strcmp(t.data.sym, "_abc") == 0);
  f = file_with_contents("abc123");
  assert(tokread_sym(f, &t));
  assert(strcmp(t.data.sym, "abc123") == 0);
  f = file_with_contents("123abc");
  assert(!tokread_sym(f, &t));
  f = file_with_contents("");
  assert(!tokread_sym(f, &t));
  f = file_with_contents("c");
  assert(tokread_sym(f, &t));
  assert(fgetc(f) == EOF);
}

test (tokread_ins) {
  tok t;
  FILE *f;
  tok_mnemonics[0] = "ld";
  tok_mnemonics[1] = "add";
  f = file_with_contents("ld");
  assert(tokread_ins(f, &t));
  assert(t.data.ins == 0);
  f = file_with_contents("add");
  assert(tokread_ins(f, &t));
  assert(t.data.ins == 1);
}

suite {
  test_case(tokread_eof);
  test_case(tokread_sym);
  test_case(tokread_ins);
  return 0;
}
  

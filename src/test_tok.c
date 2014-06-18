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
  tokread_sym(f, &t);
  assert(fgetc(f) == EOF);
  f = file_with_contents("abc def");
  tokread_sym(f, &t);
  assert(fgetc(f) == ' ');
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
  f = file_with_contents("foo");
  assert(!tokread_ins(f, &t));
  f = file_with_contents("ld");
  tokread_ins(f, &t);
  assert(fgetc(f) == EOF);
  f = file_with_contents("ld foo");
  tokread_ins(f, &t);
  assert(fgetc(f) == ' ');
}

test (tokread_reg) {
  tok t;
  FILE *f;
  tok_registers[0] = "HL";
  tok_registers[1] = "A";
  f = file_with_contents("HL");
  assert(tokread_reg(f, &t));
  assert(t.data.reg == 0);
  f = file_with_contents("A");
  assert(tokread_reg(f, &t));
  assert(t.data.reg == 1);
  f = file_with_contents("foo");
  assert(!tokread_reg(f, &t));
  f = file_with_contents("HL");
  tokread_reg(f, &t);
  assert(fgetc(f) == EOF);
  f = file_with_contents("HL foo");
  tokread_reg(f, &t);
  assert(fgetc(f) == ' ');
}

test (read_dec) {
  tok t;
  FILE *f;
  f = file_with_contents("123");
  assert(read_dec(f, &t));
  assert(t.data.num == 123);
  assert(fgetc(f) == EOF);
  f = file_with_contents("123 abc");
  assert(read_dec(f, &t));
  assert(fgetc(f) == ' ');
}

test (read_hex) {
  tok t;
  FILE *f;
  f = file_with_contents("0xABC123");
  assert(read_hex(f, &t));
  assert(t.data.num == 0xABC123);
  assert(fgetc(f) == EOF);
  f = file_with_contents("0xABC123 abc");
  assert(read_hex(f, &t));
  assert(fgetc(f) == ' ');
  f = file_with_contents("$ABC123");
  assert(read_hex(f, &t));
  assert(t.data.num == 0xABC123);
  assert(fgetc(f) == EOF);
  f = file_with_contents("$ABC123 abc");
  assert(read_hex(f, &t));
  assert(fgetc(f) == ' ');
  f = file_with_contents("ABC123h");
  assert(read_hex(f, &t));
  assert(t.data.num == 0xABC123);
  assert(fgetc(f) == EOF);
  f = file_with_contents("ABC123h abc");
  assert(read_hex(f, &t));
  assert(fgetc(f) == ' ');
}

test (read_oct) {
  tok t;
  FILE *f;
  f = file_with_contents("0123");
  assert(read_oct(f, &t));
  assert(t.data.num == 0123);
  assert(fgetc(f) == EOF);
  f = file_with_contents("0123 abc");
  assert(read_oct(f, &t));
  assert(fgetc(f) == ' ');
  f = file_with_contents("123o");
  assert(read_oct(f, &t));
  assert(t.data.num == 0123);
  assert(fgetc(f) == EOF);
  f = file_with_contents("123o abc");
  assert(read_oct(f, &t));
  assert(fgetc(f) == ' ');
}

test (read_bin) {
  tok t;
  FILE *f;
  f = file_with_contents("0b111100000001");
  assert(read_bin(f, &t));
  assert(t.data.num == 0xF01);
  assert(fgetc(f) == EOF);
  f = file_with_contents("0b111100000001 abc");
  assert(read_hex(f, &t));
  assert(fgetc(f) == ' ');
  f = file_with_contents("%111100000001");
  assert(read_bin(f, &t));
  assert(t.data.num == 0xF01);
  assert(fgetc(f) == EOF);
  f = file_with_contents("%111100000001 abc");
  assert(read_hex(f, &t));
  assert(fgetc(f) == ' ');
  f = file_with_contents("111100000001b");
  assert(read_bin(f, &t));
  assert(t.data.num == 0xF01);
  assert(fgetc(f) == EOF);
  f = file_with_contents("111100000001b abc");
  assert(read_hex(f, &t));
  assert(fgetc(f) == ' ');
}

test (read_char) {
  tok t;
  FILE *f;
  f = file_with_contents("'a'");
  assert(read_char(f, &t));
  assert(t.data.num == 'a');
  assert(fgetc(f) == EOF);
  f = file_with_contents("'a' abc");
  assert(read_char(f, &t));
  assert(fgetc(f) == ' ');
  f = file_with_contents("'\\\\'");
  assert(read_char(f, &t));
  assert(t.data.num == '\\');
  f = file_with_contents("'\\''");
  assert(read_char(f, &t));
  assert(t.data.num == '\'');
}

test (read_bool) {
  tok t;
  FILE *f;
  f = file_with_contents("true");
  assert(read_bool(f, &t));
  assert(t.data.num == 1);
  assert(fgetc(f) == EOF);
  f = file_with_contents("false abc");
  assert(read_bool(f, &t));
  assert(t.data.num == 0);
  assert(fgetc(f) == ' ');
}

suite {
  test_case(tokread_eof);
  test_case(tokread_sym);
  test_case(tokread_ins);
  test_case(tokread_reg);
  test_case(read_dec);
  test_case(read_hex);
  test_case(read_oct);
  test_case(read_bin);
  test_case(read_char);
  test_case(read_bool);
  return 0;
}
  

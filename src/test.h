#ifndef TEST_H_
#define TEST_H_

#include <stdio.h>
#include <string.h>

#define assert(a) if (!(a)) { \
  __test_info->status = FAIL; \
  fprintf(stderr, "    \033[31;1m" __FILE__ ":%d" \
      " %s assertion failed:\033[0m %s\n", __LINE__, __test_info->name, #a); \
}
#define assert_now(a) if (!(a)) { \
  __test_info->status = FAIL; \
  fprintf(stderr, "    \033[31;1m" __FILE__ ":%d" \
      " %s assertion failed:\033[0m %s\n", __LINE__, __test_info->name, #a); \
  return; \
}

#define fail(msg, ...) { \
  __test_info->status = FAIL; \
  fprintf(stderr, "    \033[31;1m" __FILE__ ":%d" \
      " %s failure:\033[0m " msg, __LINE__, __test_info->name, ##__VA_ARGS__); \
}
#define fail_now(msg, ...) { \
  __test_info->status = FAIL; \
  fprintf(stderr, "    \033[31;1m" __FILE__ ":%d" \
      " %s failure:\033[0m " msg, __LINE__, __test_info->name, ##__VA_ARGS__); \
  return; \
}

#ifdef TEST_VERBOSE
#undef assert
#undef assert_now
#define assert(a) if (!(a)) { \
  __test_info->status = FAIL; \
  fprintf(stderr, "    \033[31;1m" __FILE__ ":%d" \
      " %s assertion failed:\033[0m %s\n", __LINE__, __test_info->name, #a); \
} else { \
  fprintf(stderr, "    \033[32;1m" __FILE__ ":%d" \
      " %s assertion passed:\033[0m %s\n", __LINE__, __test_info->name, #a); \
}
#define assert_now(a) if (!(a)) { \
  __test_info->status = FAIL; \
  fprintf(stderr, "    \033[31;1m" __FILE__ ":%d" \
      " %s assertion failed:\033[0m %s\n", __LINE__, __test_info->name, #a); \
  return; \
} else { \
  fprintf(stderr, "    \033[32;1m" __FILE__ ":%d" \
      " %s assertion passed:\033[0m %s\n", __LINE__, __test_info->name, #a); \
}
#endif

#define test(name) static void __test_ ## name(test_info *__test_info)

#define suite int main(int argc, char **argv)

#define test_case(t) { \
  int run_test = argc <= 1; \
  int i; \
  if (!run_test) for (i = 1; i < argc; i++) if (0 == strcmp(argv[i], #t)) run_test = 1; \
  if (run_test) { \
    fprintf(stderr, "\033[34;1mRunning test '" #t "'\033[0m\n"); \
    test_info __test_info = { .name = #t, .status = PASS }; \
    __test_ ## t(&__test_info); \
    if (__test_info.status == PASS) { \
      fprintf(stderr, "\033[32;1mTest '" #t "' passed\033[0m\n"); \
    } else { \
      fprintf(stderr, "\033[31;1mTest '" #t "' failed\033[0m\n"); \
    } \
  } \
}

#define PASS 1
#define FAIL 2

typedef struct {
  char *name;
  int status;
} test_info;

#endif

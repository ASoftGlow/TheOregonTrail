#include "tests.h"

#define tests_start(name)        \
  static unsigned LS = __LINE__; \
  void tests_##name(unsigned sub_line_start, unsigned* failed)

#define _test_prefix \
  if (__LINE__ - LS - 1 >= sub_line_start) *failed |=

void _test_start(const char* tests_name, unsigned line_number);
void _test_fail_start(void);
void _test_fail_end(void);
void _test_success(void);

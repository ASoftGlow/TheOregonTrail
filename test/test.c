#include "test.h"
#include "ansi_codes.h"
#include <stdio.h>

void
_test_start(const char* tests_name, unsigned line_number)
{
  printf("Test %s:%i \t", tests_name, line_number);
  fflush(stdout);
}

void
_test_fail_start(void)
{
  puts(ANSI_COLOR_RED "Fail!" ANSI_COLOR_RESET);
}

void
_test_fail_end(void)
{
  puts(ANSI_COLOR_RESET);
  fflush(stdout);
}

void
_test_success(void)
{
  puts(ANSI_COLOR_GREEN "Success." ANSI_COLOR_RESET);
  fflush(stdout);
}

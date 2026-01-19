#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ansi_codes.h"
#include "formatted_lines.h"
#include "input.h"
#include "test.h"

typedef struct ExpectedLine
{
  unsigned length, display_length;
} ExpectedLine;

static void
_test(const char* text, unsigned width, unsigned expected_size, ExpectedLine* expected_lines, unsigned line_number)
{
  printf("Test wrapping:%i \t", line_number);
  fflush(stdout);

  FormattedLines lines = wrapText(text, width, NULL);
  if (!(formatted_lines_size(lines) == expected_size)) goto fail;
  FormattedLinesIterator it = formatted_lines_it(lines, 0);
  FormattedLine* line = formatted_lines_it_cur(&it);
  do {
    if (!(line->length == expected_lines->length && line->display_length == expected_lines->display_length))
    {
    fail:
      printf("Fail!\n  %i\t", width);
      do {
        if (*text == ESC_CHAR) putchar('$');
        else putchar(*text);
      } while (*++text);
      putchar('\n');
      return;
    }
  } while (++expected_lines, (line = formatted_lines_it_next(&it)));
  formatted_lines_free(lines);
  puts("Success.");
}

#define test_start(name)         \
  static unsigned LS = __LINE__; \
  void test_##name(void)
#define test(text, width, ...)                                                                                      \
  _test(                                                                                                            \
      text, width, sizeof((ExpectedLine[]){ __VA_ARGS__ }) / sizeof(ExpectedLine), (ExpectedLine[]){ __VA_ARGS__ }, \
      __LINE__ - LS - 1                                                                                             \
  )

test_start(wrapping)
{
  test("x", 10, { 1, 1 });
  test("x", 1, { 1, 1 });
  test("xxx" ANSI_COLOR_RESET "xxx", 10, { 6 + strlen(ANSI_COLOR_RESET), 6 });
  test("xxx" ANSI_COLOR_RESET "xxx", 6, { 6 + strlen(ANSI_COLOR_RESET), 6 });
  test("xxx" ANSI_COLOR_RESET, 10, { 3 + strlen(ANSI_COLOR_RESET), 3 });
  test("xxx" ANSI_COLOR_RESET, 3, { 3 + strlen(ANSI_COLOR_RESET), 3 });
  test(ANSI_COLOR_RESET "xxx", 10, { 3 + strlen(ANSI_COLOR_RESET), 3 });
  test(ANSI_COLOR_RESET "xxx", 3, { 3 + strlen(ANSI_COLOR_RESET), 3 });
  test("aaaa bbbbb", 12, { 10, 10 });
  test("aaaa bbbbb", 10, { 10, 10 });
  test("aaaa bbbbb", 8, { 4, 4 }, { 5, 5 });
  test("12345", 2, { 2, 2 }, { 3, 3 });
}

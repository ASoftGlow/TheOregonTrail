#include <stdio.h>
#include <string.h>

#include "ansi_codes.h"
#include "formatted_lines.h"
#include "test.h"

typedef struct ExpectedLine
{
  unsigned length, display_length;
} ExpectedLine;

static Coord c[9];

static unsigned
_test(const char* text, unsigned width, unsigned expected_size, ExpectedLine* expected_lines, unsigned line_number)
{
  _test_start("wrapping", line_number);

  FormattedLines lines = wrapText(text, width, &(WrapLineOptions){ .captures = c });
  if (!(formatted_lines_size(lines) == expected_size)) goto fail;
  FormattedLinesIterator it = formatted_lines_it(lines, 0);
  FormattedLine* line = formatted_lines_it_cur(&it);
  do {
    if (!(line->length == expected_lines->length && line->display_length == expected_lines->display_length
          && line->text[line->length] == '\0'))
    {
    fail:
      _test_fail_start();
      printf("  %i\t`", width);
      put_esc(text);
      puts("`\n");
      fl_pls(lines);
      _test_fail_end();
      return 1;
    }
  } while (++expected_lines, (line = formatted_lines_it_next(&it)));
  formatted_lines_free(lines);
  _test_success();
  return 0;
}

#define test(text, width, ...)                                                                                      \
  _test_prefix _test(                                                                                               \
      text, width, sizeof((ExpectedLine[]){ __VA_ARGS__ }) / sizeof(ExpectedLine), (ExpectedLine[]){ __VA_ARGS__ }, \
      __LINE__ - LS - 1                                                                                             \
  )

tests_start(wrapping)
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
  test("aaaa bbbbb", 9, { 4, 4 }, { 5, 5 });
  test("aaaa bbbbb", 8, { 4, 4 }, { 5, 5 });
  test("aaa bbbb ccc dddd eeeeee", 10, { 8, 8 }, { 8, 8 }, { 6, 6 });
  test("12345", 2, { 2, 2 }, { 2, 2 }, { 1, 1 });
  test(
      "What is the first name of the wagon leader? " ANSI_CURSOR_SAVE "xxxxxxxxxx", 50, { 43, 43 },
      { 10 + strlen(ANSI_CURSOR_SAVE), 10 }
  );
  test("How many yokes? 12", 20, { 18, 18 });
  test("How many yokes? " CAPTURE_STRING "12", 20, { 18, 18 });
  test("How many " CAPTURE_STRING "yokes? " CAPTURE_STRING PH_STR PH_STR, 20, { 18, 18 });
  test("How many " CAPTURE_STRING "yokes? " CAPTURE_STRING PH_STR PH_STR, 14, { 8, 8 }, { 9, 9 });
}

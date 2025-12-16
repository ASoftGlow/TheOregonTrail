#pragma once
#include <stddef.h>

#include "ansi_codes.h"
#include "base.h"

// @brief Signifies a position to capture
#define CAPTURE_CHAR   '\5'
// @brief Signifies a position to capture
#define CAPTURE_STRING "\5"

typedef enum
{
  WRAPLINEKIND_LTR,
  WRAPLINEKIND_RTL,
  WRAPLINEKIND_CENTER,
  WRAPLINEKIND_NONE,
} WrapLineKind;

typedef struct FormattedLine
{
  byte length;
  byte display_length;
  WrapLineKind kind : 8;
  char text[];
} FormattedLine;

// void pointer to disallow array element access -- use formatted_lines_at
typedef void* FormattedLines;

CHECK_RETURN FormattedLines formatted_lines_create(size_t capacity);
CHECK_RETURN FormattedLines formatted_lines_add(FormattedLines lines, FormattedLine line, const char* text);
FormattedLine* formatted_lines_at(const FormattedLines lines, size_t i);
FormattedLine* formatted_lines_front(const FormattedLines lines);
FormattedLine* formatted_lines_back(const FormattedLines lines);
size_t formatted_lines_size(const FormattedLines lines);
void formatted_lines_clear(FormattedLines lines);
#define formatted_lines_free free

typedef struct WrapLineOptions
{
  Coord* captures;
  byte captures_count;
  FormattedLines lines;
  byte* added_count;
  WrapLineKind kind;
} WrapLineOptions;

CHECK_RETURN FormattedLines addLine(FormattedLines lines, const char* text, WrapLineKind kind);
CHECK_RETURN FormattedLines addNewline(FormattedLines lines);
CHECK_RETURN FormattedLines addBar(FormattedLines lines, char c, enum Color color);
CHECK_RETURN FormattedLines justifyLineWL(FormattedLines lines, const char* text1, const char* text2, byte width);
void indentLines(FormattedLines lines, size_t begin_i, size_t end_i, byte amount);

// @param options - is optional
CHECK_RETURN FormattedLines wrapText(const char* text, int width, WrapLineOptions* options);

/**
 * @brief Convert a string to a single line
 */
CHECK_RETURN FormattedLines textToLines(const char* text);

/**
 * @brief Convert a string to a single line
 */
CHECK_RETURN FormattedLines textToLinesWL(FormattedLines lines, const char* text);

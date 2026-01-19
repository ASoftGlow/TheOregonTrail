#pragma once
#include <stddef.h>

#include "base.h"

// Signifies a position to capture
#define CAPTURE_CHAR   '\5'
#define CAPTURE_STRING "\5"
static_assert(sizeof(CAPTURE_STRING) == 2, "Capture string too long");

typedef enum
{
  WRAPLINEKIND_LTR,
  WRAPLINEKIND_RTL,
  WRAPLINEKIND_CENTER,
  WRAPLINEKIND_NONE,
} WrapLineKind;

typedef struct FormattedLine
{
  byte length; // excludes null terminator
  byte display_length;
  WrapLineKind kind : 8;
  char text[];
} FormattedLine;

/*
 * Lines cannot be moved or be resized
 * void pointer to disallow array element access -- use formatted_lines_at
 */
typedef void* FormattedLines;
typedef unsigned FormattedLinesSize_t;

/**
 * Make sure not to modify lines after, as the address may change
 */
typedef struct
{
  const FormattedLines lines;
  unsigned _pos; // byte position
} FormattedLinesIterator;

// @see FormattedLines
CHECK_RETURN FormattedLines formatted_lines_new(void);
CHECK_RETURN FormattedLines formatted_lines_add(FormattedLines lines, FormattedLine line);
// undefined if lines is empty
FormattedLine* formatted_lines_front(const FormattedLines lines);
// undefined if lines is empty
FormattedLine* formatted_lines_back(const FormattedLines lines);
/**
 * @see FormattedLinesIterator
 * @param start_index passed to formatted_lines_seek
 */
FormattedLinesIterator formatted_lines_it(const FormattedLines lines, unsigned start_index);
// undefined if lines is empty
FormattedLine* formatted_lines_it_cur(const FormattedLinesIterator* it);
// returns NULL if at end
FormattedLine* formatted_lines_it_next(FormattedLinesIterator* it);
// @param index < size, -1 for last
FormattedLine* formatted_lines_it_seek(FormattedLinesIterator* it, unsigned index);
// number of lines
FormattedLinesSize_t formatted_lines_size(const FormattedLines lines);
// empties count, but does not free
void formatted_lines_clear(FormattedLines lines);
void formatted_lines_free(FormattedLines lines);

typedef struct WrapLineOptions
{
  Coord* captures; // with size at least captures_count
  byte captures_count;
  FormattedLines lines; // optional existing lines to append to
  WrapLineKind kind;
} WrapLineOptions;

CHECK_RETURN FormattedLines addLine(FormattedLines lines, const char* text, WrapLineKind kind);
CHECK_RETURN FormattedLines addEmptyLine(FormattedLines lines, byte length, WrapLineKind kind);
CHECK_RETURN FormattedLines addNewline(FormattedLines lines);
CHECK_RETURN FormattedLines justifyLineWL(FormattedLines lines, const char* text1, const char* text2, byte width);
// frees given lines and returns new ones
CHECK_RETURN FormattedLines indentLines(FormattedLines lines, byte amount, FormattedLines new_lines);

// @param options - is optional
CHECK_RETURN FormattedLines wrapText(const char* text, unsigned width, WrapLineOptions* options);

/**
 * Convert a string to a single line
 */
CHECK_RETURN FormattedLines textToLines(const char* text);

/**
 * Convert a string to a single line
 */
CHECK_RETURN FormattedLines textToLinesWL(FormattedLines lines, const char* text);

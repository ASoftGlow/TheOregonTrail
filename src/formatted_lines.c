#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include "formatted_lines.h"
#include "input.h"
#include "utils.h"

typedef struct
{
  size_t size, capacity;
} Metadata;

extern byte SCREEN_WIDTH, DIALOG_CONTENT_WIDTH;
static const char EMPTY_STRING[] = "";

static struct WrapLineOptions DEFAULT_WrapLineOptions = {
  .kind = WRAPLINEKIND_LTR,
};

#define formatted_line_size                       (sizeof(Metadata) + SCREEN_WIDTH)
#define __formatted_lines_capacity_size(capacity) ((sizeof(Metadata) + (capacity) * formatted_line_size))

CHECK_RETURN FormattedLines
formatted_lines_create(size_t capacity)
{
  Metadata* lines = malloc(__formatted_lines_capacity_size(capacity));
  assert(lines);
  lines->size = 0;
  lines->capacity = capacity;

  return lines;
}

FormattedLine*
formatted_lines_at(const FormattedLines lines, size_t i)
{
  return (FormattedLine*)((byte*)lines + __formatted_lines_capacity_size(i));
}

FormattedLine*
formatted_lines_front(const FormattedLines lines)
{
  return formatted_lines_at(lines, 0);
}

FormattedLine*
formatted_lines_back(const FormattedLines lines)
{
  return formatted_lines_at(lines, formatted_lines_size(lines) - 1);
}

CHECK_RETURN FormattedLines
formatted_lines_add(FormattedLines lines, FormattedLine line, const char* text)
{
  lines = realloc(lines, __formatted_lines_capacity_size(formatted_lines_size(lines) + 1));
  assert(lines);
  ((Metadata*)lines)->size++;
  FormattedLine* last = formatted_lines_back(lines);
  *last = line;
  if (text) strcpy(last->text, text);
  return lines;
}

CHECK_RETURN size_t
formatted_lines_size(const FormattedLines lines)
{
  return ((Metadata*)lines)->size;
}

void
formatted_lines_clear(FormattedLines lines)
{
  ((Metadata*)lines)->size = 0;
}

CHECK_RETURN FormattedLines
addNewline(FormattedLines lines)
{
  return formatted_lines_add(lines, (struct FormattedLine){ .kind = WRAPLINEKIND_LTR }, EMPTY_STRING);
}

CHECK_RETURN FormattedLines
addLine(FormattedLines lines, const char* text, WrapLineKind kind)
{
  FormattedLine line = {
    .length = (byte)strlen(text),
    .display_length = (byte)_strlen_iae(text),
    .kind = kind,
  };
  return formatted_lines_add(lines, line, text);
}

CHECK_RETURN FormattedLines
wrapText(const char* text, int width, WrapLineOptions* options)
{
  if (!options)
  {
    options = &DEFAULT_WrapLineOptions;
  }

  byte start = 0;
  FormattedLines lines = options->lines;
  if (options->lines)
  {
    start = (byte)formatted_lines_size(lines);
  }
  else
  {
    lines = formatted_lines_create(1);
  }
  byte l = start;
  lines = formatted_lines_add(lines, (struct FormattedLine){ .kind = options->kind }, NULL);

  int i = -1;
  bool is_escaping = 0;
  byte escaped_chars = 0;
  int last_break = 0;
  int capture_i = 0;

  while (text[++i])
  {
    FormattedLine* line = formatted_lines_at(lines, l);
    if (text[i] == '\n')
    {
    newline:
      line->display_length = line->length - escaped_chars;
      line->text[line->length++] = 0;
      ++l;
      lines = formatted_lines_add(lines, (struct FormattedLine){ .kind = options->kind }, NULL);
      escaped_chars = 0;
      continue;
    }
    else if (text[i] == ESC_CHAR) is_escaping = 1;
    else if (text[i] == ' ') last_break = i;
    else if (text[i] == CAPTURE_CHAR && options->captures)
    {
      options->captures[capture_i].y = l;
      options->captures[capture_i++].x = line->length;
      continue;
    }

    if (is_escaping)
    {
      ++escaped_chars;
      if (isalpha(text[i])) is_escaping = 0;
    }
    line->text[line->length++] = text[i];
    if (line->length - escaped_chars == width)
    {
      line->text[line->length -= i - last_break + 1] = 0;
      i = last_break;
      goto newline;
    }
  }
  FormattedLine* line = formatted_lines_at(lines, l);
  line->display_length = line->length - escaped_chars;
  line->text[line->length++] = 0;

  if (options->added_count) *options->added_count = l - start + (byte)1;
  options->captures_count = capture_i;
  return lines;
}

CHECK_RETURN FormattedLines
justifyLineWL(FormattedLines lines, const char* text_l, const char* text_r, byte width)
{
  struct FormattedLine line = {
    .length = width,
    .display_length = width,
    .kind = WRAPLINEKIND_NONE,
  };
  lines = formatted_lines_add(lines, line, NULL);
  FormattedLine* last = formatted_lines_back(lines);

  const byte len2 = (byte)strlen(text_r);
  byte pos = (byte)strlen(text_l);
  memcpy(last->text, text_l, pos);
  while (pos + len2 < width) last->text[pos++] = ' ';
  memcpy(last->text + pos, text_r, (size_t)len2);
  pos += len2;
  while (pos < width) last->text[pos++] = ' ';
  last->text[pos] = 0;

  return lines;
}

void
indentLines(FormattedLines lines, size_t begin_i, size_t end_i, byte amount)
{
  while (begin_i < end_i)
  {
    FormattedLine* line = formatted_lines_at(lines, begin_i);
    assert(line->length + amount <= SCREEN_WIDTH);
    memmove(line->text + amount, line->text, line->length);
    memset(line->text, ' ', amount);
    line->length += amount;
    line->display_length += amount;
    ++begin_i;
  }
}

CHECK_RETURN FormattedLines
textToLines(const char* text)
{
  FormattedLines lines = formatted_lines_create(1);
  return textToLinesWL(lines, text);
}

CHECK_RETURN FormattedLines
textToLinesWL(FormattedLines lines, const char* text)
{
  char* _text = strdup(text);
  char* line_text;

  // split by newline
  line_text = strtok(_text, "\n");
  while (line_text)
  {
    FormattedLine line = {
      .length = (byte)strlen(line_text),
      .display_length = line.length,
      .kind = WRAPLINEKIND_LTR,
    };
    lines = formatted_lines_add(lines, line, NULL);
    FormattedLine* last = formatted_lines_back(lines);
    memcpy(last->text, line_text, line.length);
    last->text[line.length] = 0;

    line_text = strtok(0, "\n");
  }
  free(_text);
  return lines;
}

CHECK_RETURN FormattedLines
addBar(FormattedLines lines, char c, enum Color color)
{
  byte l = DIALOG_CONTENT_WIDTH;
  VLA(char, buf, l + 16);

  strcpy(buf, ANSI_COLORS[color]);
  char* end = buf + strlen(ANSI_COLORS[color]);
  memset(end, c, l);
  end[l] = '\0';
  return addLine(lines, buf, WRAPLINEKIND_LTR);
}

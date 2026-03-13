#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include "formatted_lines.h"
#include "input.h"
#include "utils.h"

#pragma region Data structure implementation

// FormattedLines beginning structure
typedef struct
{
  // store last to allow fast location of last line
  byte last_length;
  unsigned short size; // number of lines
  unsigned total_size; // byte size
} Im;

#define formatted_line_size(line)   (sizeof(FormattedLine) + (line).length + sizeof((char)0))
#define formatted_line_back_pos(im) im->total_size - (sizeof(FormattedLine) + im->last_length + sizeof((char)0))

CHECK_RETURN FormattedLines
formatted_lines_new(void)
{
  Im* lines = malloc(sizeof(Im));
  assert(lines);
  formatted_lines_clear(lines);

  return lines;
}

FormattedLine*
formatted_lines_front(const FormattedLines lines)
{
  assert(((Im*)lines)->size);
  return (FormattedLine*)((byte*)lines + sizeof(Im));
}

FormattedLine*
formatted_lines_back(const FormattedLines lines)
{
  Im* im = lines;
  assert(im->size);
  return (FormattedLine*)((byte*)lines + formatted_line_back_pos(im));
}

FormattedLinesIterator
formatted_lines_it(const FormattedLines lines, unsigned index)
{
  assert(lines);
  FormattedLinesIterator it = { .lines = lines };
  formatted_lines_it_seek(&it, index);
  return it;
}

FormattedLine*
formatted_lines_it_cur(const FormattedLinesIterator* it)
{
  return (FormattedLine*)((byte*)it->lines + it->_pos);
}

FormattedLine*
formatted_lines_it_next(FormattedLinesIterator* it)
{
  assert(((Im*)it->lines)->total_size >= it->_pos + formatted_line_size(*formatted_lines_it_cur(it)));
  if (((Im*)it->lines)->total_size == it->_pos + formatted_line_size(*formatted_lines_it_cur(it))) return NULL;
  it->_pos += formatted_line_size(*formatted_lines_it_cur(it));
  return formatted_lines_it_cur(it);
}

FormattedLine*
formatted_lines_it_seek(FormattedLinesIterator* it, unsigned index)
{
  Im* im = it->lines;
  if (index == (unsigned)-1)
  {
    assert(im->size);
    it->_pos = formatted_line_back_pos(im);
  }
  else
  {
    assert(index < im->size);
    it->_pos = sizeof(Im);
    while (index--)
    {
      formatted_lines_it_next(it);
    }
  }
  return formatted_lines_it_cur(it);
}

CHECK_RETURN FormattedLines
formatted_lines_add(FormattedLines lines, FormattedLine line)
{
  Im* im = lines;
  im->size++;
  im->last_length = line.length;
  im->total_size += formatted_line_size(line);
  lines = realloc(lines, im->total_size);
  assert(lines);

  FormattedLine* last = formatted_lines_back(lines);
  *last = line;
  last->text[0] = '\0';
  return lines;
}

CHECK_RETURN FormattedLinesSize_t
formatted_lines_size(const FormattedLines lines)
{
  return ((Im*)lines)->size;
}

void
formatted_lines_clear(FormattedLines lines)
{
  Im* im = lines;
  im->size = 0;
  im->last_length = 0;
  im->total_size = sizeof(Im);
}

void
formatted_lines_free(FormattedLines lines)
{
  free(lines);
}

#pragma endregion

static struct WrapLineOptions DEFAULT_WrapLineOptions = {
  .kind = WRAPLINEKIND_LTR,
};

CHECK_RETURN FormattedLines
addLine(FormattedLines lines, const char* text, WrapLineKind kind)
{
  FormattedLine line = {
    .length = (byte)strlen(text),
    .display_length = (byte)_strlen_iae(text),
    .kind = kind,
  };
  lines = formatted_lines_add(lines, line);
  memcpy(formatted_lines_back(lines)->text, text, line.length + 1);
  return lines;
}

CHECK_RETURN FormattedLines
addEmptyLine(FormattedLines lines, byte length, WrapLineKind kind)
{
  return formatted_lines_add(lines, (FormattedLine){ .length = length, .kind = kind });
}

CHECK_RETURN FormattedLines
addNewline(FormattedLines lines)
{
  return addEmptyLine(lines, 0, WRAPLINEKIND_LTR);
}

static FormattedLines
_wrapText_newline(FormattedLines lines, const FormattedLine line, const char* text, unsigned skipped_chars)
{
  lines = formatted_lines_add(lines, line);
  FormattedLine* last = formatted_lines_back(lines);
  unsigned j = 0;
  for (unsigned i = 0; i < line.length + skipped_chars; i++)
  {
    assert(text[i]);
    switch (text[i])
    {
    case CAPTURE_CHAR:     continue;
    case PLACEHOLDER_CHAR: last->text[j++] = ' '; break;
    default:               last->text[j++] = text[i]; break;
    }
  }
  last->text[j] = '\0';
  return lines;
}

CHECK_RETURN FormattedLines
wrapText(const char* text, unsigned width, WrapLineOptions* options)
{
  assert(text && text[0]); // don't wrap an empty string
  if (!options) options = &DEFAULT_WrapLineOptions;

  FormattedLines lines = options->lines;
  if (!options->lines) lines = formatted_lines_new();

  const char *c = text - 1, *line_start = text;
  bool is_escaping = 0, break_next = 0;
  byte escaped_chars = 0, skipped_chars = 0;

  struct
  {
    const char* pos;
    unsigned escaped_chars;
    unsigned skipped_chars;
  } last_break = { c };

  options->captures_count = 0;

  FormattedLine line = { .kind = options->kind };

  while (1)
  {
    switch (*++c)
    {
    case '\0':
      if (!line.length) return lines;
      FALLTHROUGH;
    case '\n':
      line.display_length = line.length - escaped_chars;
      lines = _wrapText_newline(lines, line, line_start, skipped_chars);
      if (*c)
      {
        line_start = c + 1;
        line.length = 0;
        escaped_chars = 0;
        skipped_chars = 0;
        continue;
      }
      // if at end
#ifdef DEBUG
      assert(!is_escaping);

      FormattedLinesIterator it = formatted_lines_it(lines, 0);
      FormattedLine* l = formatted_lines_it_cur(&it);
      do {
        assert(l->display_length <= l->length);
        assert(l->display_length <= width);
        for (char* c = l->text; *c; c++)
        {
          assert(*c != CAPTURE_CHAR && *c != PLACEHOLDER_CHAR);
        }
      } while ((l = formatted_lines_it_next(&it)));
#endif
      return lines;

    case ESC_CHAR:
      assert(!is_escaping);
      is_escaping = 1;
      break;

    case ' ':
      last_break.pos = c;
      last_break.escaped_chars = escaped_chars;
      last_break.skipped_chars = skipped_chars;
      break;

    case CAPTURE_CHAR:
      assert(options->captures && !is_escaping);
      options->captures[options->captures_count].y = formatted_lines_size(lines);
      options->captures[options->captures_count++].x = line.length;
      ++skipped_chars;
      continue;
    }

    ++line.length;
    if (is_escaping)
    {
      ++escaped_chars;
      if (isalpha(*c) || (c > text && *(c - 1) == ESC_CHAR && isdigit(*c))) is_escaping = 0;
    }
    // only check when the display length changes
    else if ((unsigned)(line.length - escaped_chars) >= width)
    {
      if (break_next)
      {
        break_next = 0;

        if (c - last_break.pos + 1 - escaped_chars >= width)
        {
          FormattedLine break_line = {
            .length = width + escaped_chars,
            .display_length = width,
          };
          lines = _wrapText_newline(lines, break_line, line_start, skipped_chars);
          line.length = 1; // probablem here
          escaped_chars = 0;
          // TODO: skipped_chars
          line_start = c;
          continue;
        }

        FormattedLine break_line = {
          .length = last_break.pos - line_start,
          .display_length = break_line.length - last_break.escaped_chars,
        };
        lines = _wrapText_newline(lines, break_line, line_start, last_break.skipped_chars);
        line.length -= break_line.length + 1;
        escaped_chars -= last_break.escaped_chars;
        skipped_chars -= last_break.skipped_chars;
        line_start = last_break.pos + 1;
      }
      else break_next = 1;
    }
  }
}

CHECK_RETURN FormattedLines
justifyLineWL(FormattedLines lines, const char* text_l, const char* text_r, byte width)
{
  struct FormattedLine line = {
    .length = width,
    .display_length = width,
    .kind = WRAPLINEKIND_NONE,
  };
  lines = formatted_lines_add(lines, line);
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

CHECK_RETURN FormattedLines
indentLines(FormattedLines lines, byte amount, FormattedLines new_lines)
{
  FormattedLinesIterator it = formatted_lines_it(lines, 0);
  FormattedLine* line = formatted_lines_it_cur(&it);
  do {
    // copy each line, prefixing indent
    FormattedLine new_line = {
      .kind = line->kind,
      .length = line->length + amount,
      .display_length = line->display_length + amount,
    };
    new_lines = formatted_lines_add(new_lines, new_line);
    char* new_text = formatted_lines_back(new_lines)->text;
    memset(new_text, ' ', amount);
    memcpy(new_text + amount, line->text, line->length + 1);
  } while ((line = formatted_lines_it_next(&it)));

  formatted_lines_free(lines);
  return new_lines;
}

CHECK_RETURN FormattedLines
textToLines(const char* text)
{
  FormattedLines lines = formatted_lines_new();
  return textToLinesWL(lines, text);
}

CHECK_RETURN FormattedLines
textToLinesWL(FormattedLines lines, const char* in_text)
{
  char* text = strdup(in_text);

  // split by newline
  char* line_text = strtok(text, "\n");
  while (line_text)
  {
    FormattedLine line = {
      .length = (byte)strlen(line_text),
      .display_length = line.length,
      .kind = WRAPLINEKIND_LTR,
    };
    lines = formatted_lines_add(lines, line);
    FormattedLine* last = formatted_lines_back(lines);
    memcpy(last->text, line_text, line.length);
    last->text[line.length] = '\0';

    line_text = strtok(0, "\n");
  }
  free(text);
  return lines;
}

#ifdef DEBUG
void
put_esc(const char* text)
{
  while (*text)
  {
    switch (*text)
    {
    case ESC_CHAR:         putchar('$'); break;
    case PLACEHOLDER_CHAR: putchar('@'); break;
    default:               putchar(*text);
    }
    ++text;
  }
}

void
fl_pls(const FormattedLines lines)
{
  printf("%p size: %i\n", lines, formatted_lines_size(lines));
  if (formatted_lines_size(lines))
  {
    FormattedLinesIterator it = formatted_lines_it(lines, 0);
    FormattedLine* line = formatted_lines_it_cur(&it);
    do {
      printf("  len: %i:%i\t txt: `", line->length, line->display_length);
      put_esc(line->text);
      puts("`");
    } while ((line = formatted_lines_it_next(&it)));
  }
}
#endif

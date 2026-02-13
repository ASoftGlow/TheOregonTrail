#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "input.h"
#include "tui.h"
#include "utils.h"

// Provides y position of first line and last line
struct ChoiceInfo
{
  byte start, end;
};

void drawChoice(
    const struct ChoiceDialogChoice* choices, const FormattedLines lines, const struct ChoiceInfo* choices_info, Coord offset,
    int index, bool selected
);

byte SCREEN_WIDTH;
byte SCREEN_HEIGHT;
byte DIALOG_CONTENT_WIDTH;

static const char PRESS_SPACE[] = ANSI_COLOR_GREEN "Press SPACE BAR to continue" ANSI_COLOR_RESET;

static struct BoxOptions DEFAULT_BoxOptions = {
  .paddingX = DIALOG_PADDING_X,
  .paddingY = DIALOG_PADDING_Y,
};
static struct DialogOptions DEFAULT_DialogOptions = {
  .color = COLOR_YELLOW,
};

#ifdef TOT_ASCII
const BoxCharCollection BOX_CHAR_BORDERS[__BORDER_END] = {
  { .DR = '@', .DL = '@', .H = '=', .V = '|', .UR = '@', .UL = '@' },
  { .DR = '+', .DL = '+', .H = '-', .V = '|', .UR = '+', .UL = '+' },
};
#else
const BoxCharCollection BOX_CHAR_BORDERS[__BORDER_END] = {
  { .DR = "\u2554", .DL = "\u2557", .H = "\u2550", .V = "\u2551", .UR = "\u255A", .UL = "\u255D" },
  { .DR = "\u250C", .DL = "\u2510", .H = "\u2500", .V = "\u2502", .UR = "\u2514", .UL = "\u2518" },
};
#endif

static QKeyCallbackReturn
dialogInputCallback(int key, QKeyInputValue value, va_list args)
{
  int* cur_pos = va_arg(args, int*);
  const int choices_size = va_arg(args, const int);
  const struct ChoiceDialogChoice* choices = va_arg(args, const struct ChoiceDialogChoice*);
  const FormattedLines lines = va_arg(args, const FormattedLines);
  const struct ChoiceInfo* choices_info = va_arg(args, const struct ChoiceInfo*);
  const Coord offset = va_arg(args, const Coord);
  const Coord end = va_arg(args, const Coord);

  switch (key)
  {
  case ETR_CHAR:
    if (*cur_pos != -1)
    {
      if (choices[*cur_pos].disabled) break;
      putsn(ANSI_CURSOR_SHOW);
      return (QKeyCallbackReturn){ QKEY_BEHAVIOR_END, { .number = *cur_pos + 1 } };
    }
    else if (value.number && choices[value.number - 1].disabled)
    {
      return (QKeyCallbackReturn){ QKEY_BEHAVIOR_IGNORE };
    }
    break;

  case ESC_CHAR:
    if (*cur_pos > -1)
    {
      drawChoice(choices, lines, choices_info, offset, *cur_pos, 0);
      *cur_pos = -1;
      setCursorPos(end.x, end.y);
      putsn(ANSI_CURSOR_SHOW);
      fflush(stdout);
      escape_combo = 0;
    }
    break;

  case KEY_ARROW_DOWN:
    if (*cur_pos != -1) drawChoice(choices, lines, choices_info, offset, *cur_pos, 0);
    else putsn(ANSI_CURSOR_HIDE);

    if (*cur_pos == choices_size - 1) *cur_pos = -1;
    drawChoice(choices, lines, choices_info, offset, ++*cur_pos, 1);
    break;

  case KEY_ARROW_UP:
    if (*cur_pos != -1) drawChoice(choices, lines, choices_info, offset, *cur_pos, 0);
    else
    {
      *cur_pos = choices_size;
      putsn(ANSI_CURSOR_HIDE);
    }
    if (*cur_pos == 0) *cur_pos = choices_size;
    drawChoice(choices, lines, choices_info, offset, --*cur_pos, 1);
    break;

  case KEY_PAGE_DOWN:
    if (*cur_pos == -1 || *cur_pos == choices_size - 1) break;
    drawChoice(choices, lines, choices_info, offset, *cur_pos, 0);
    *cur_pos = choices_size - 1;
    drawChoice(choices, lines, choices_info, offset, *cur_pos, 1);
    break;

  case KEY_PAGE_UP:
    if (*cur_pos <= 0) break;
    drawChoice(choices, lines, choices_info, offset, *cur_pos, 0);
    *cur_pos = 0;
    drawChoice(choices, lines, choices_info, offset, *cur_pos, 1);
    break;
  }

  return (QKeyCallbackReturn){ *cur_pos == -1 ? QKEY_BEHAVIOR_NORMAL : QKEY_BEHAVIOR_IGNORE };
}

int
showChoiceDialog(const char* text, unsigned choices_size, const struct ChoiceDialogChoice choices[], DialogOptions* options)
{
  FormattedLines lines = wrapText(text, DIALOG_CONTENT_WIDTH, NULL);
  lines = addNewline(lines);
  return showChoiceDialogWL(lines, choices_size, choices, options);
}

int
showChoiceDialogWL(
    FormattedLines lines, unsigned choices_size, const struct ChoiceDialogChoice choices[], DialogOptions* options
)
{
  if (!options) options = &DEFAULT_DialogOptions;
  else if (!options->color) options->color = DEFAULT_DialogOptions.color;
  if (!lines) lines = formatted_lines_new();

  const byte padding_y = options->noPaddingY ? 0 : DIALOG_PADDING_Y;
  struct ChoiceInfo* choices_info = (struct ChoiceInfo*)malloc(sizeof(struct ChoiceInfo) * choices_size);
  assert(choices_info);

  for (unsigned i = 0; i < choices_size; i++)
  {
    VLA(char, str, strlen(choices[i].name) + 8);
    sprintf(str, "%i. %s", i + 1, choices[i].name);

    choices_info[i].start = (byte)formatted_lines_size(lines);
    if (choices[i].disabled)
    {
      // add coloration
      FormattedLines choice_lines = wrapText(str, DIALOG_CONTENT_WIDTH, NULL);
      FormattedLinesIterator it = formatted_lines_it(choice_lines, 0);
      FormattedLine* line = formatted_lines_it_cur(&it);
      do {
        VLA(char, line_str, line->length + (strlen(ANSI_COLOR_GRAY) + strlen(ANSI_COLOR_RESET)) + 1);
        sprintf(line_str, ANSI_COLOR_GRAY "%s" ANSI_COLOR_RESET, line->text);
        lines = addLine(lines, line_str, line->kind);
      } while ((line = formatted_lines_it_next(&it)));
      formatted_lines_free(choice_lines);
    }
    else
    {
      lines = wrapText(str, DIALOG_CONTENT_WIDTH, &(struct WrapLineOptions){ .lines = lines });
    }
    choices_info[i].end = (byte)formatted_lines_size(lines) - 1;
  }
  lines = addNewline(lines);

  const char prompt[] = "What is your choice? ";
  lines = addLine(lines, prompt, WRAPLINEKIND_LTR);
  Coord capture = { sizeof(prompt) + DIALOG_PADDING_X, formatted_lines_size(lines) + padding_y };

  clearStdout();
  putsn(ANSI_CURSOR_SHOW);
  drawBoxWL(
      lines, DIALOG_WIDTH, BORDER_DOUBLE,
      &(struct BoxOptions){
          .title = options->title,
          .color = options->color,
          .paddingX = DIALOG_PADDING_X,
          .paddingY = padding_y,
          .do_not_free = 1,
      }
  );
  putsn(ANSI_CURSOR_SAVE);
  setCursorPos(capture.x, capture.y);
  fflush(stdout);

  int cur_pos = -1;
  const Coord offset = { .x = 1 + DIALOG_PADDING_X, .y = 1 + padding_y };
  const int num = getNumberInput(
      1, choices_size, 0, &dialogInputCallback, &cur_pos, choices_size, choices, lines, choices_info, offset, capture
  );

  formatted_lines_free(lines);
  free(choices_info);

  if (num < 0) return -1;
  return num - 1;
}

void
drawChoice(
    const struct ChoiceDialogChoice* choices, const FormattedLines lines, const struct ChoiceInfo* choices_info, Coord offset,
    int index, bool selected
)
{
  (void)choices;

  FormattedLinesIterator it = formatted_lines_it(lines, choices_info[index].start);
  escape_combo = 0;

  for (byte i = choices_info[index].start;; i++)
  {
    if (selected) putsn(ANSI_SELECTED);
    setCursorPos(offset.x, offset.y + i);
    putsn(formatted_lines_it_cur(&it)->text);
    if (i == choices_info[index].end) break;
    formatted_lines_it_next(&it);
  }
  if (selected) putsn(ANSI_COLOR_RESET);
  fflush(stdout);
}

FormattedLines
wrapBox(const char* text, unsigned width, BoxOptions* options)
{
  struct WrapLineOptions wrap_options = { .captures = options->captures };
  FormattedLines lines = wrapText(text, width - options->paddingX * 2 - 2, &wrap_options);
  options->captures_count = wrap_options.captures_count;
  return lines;
}

void
drawBox(const char* text, unsigned width, enum BorderStyle border, BoxOptions* options)
{
  drawBoxWL(wrapBox(text, width, options), width, border, options);
}

#define border BOX_CHAR_BORDERS[border_style]

static inline void
drawBoxTitle(unsigned width, const char* title, enum BorderStyle border_style, enum Color color)
{
  if (color) putsn(ANSI_COLORS[color]);
  putBoxChar(border.DR);
  if (title)
  {
    const int l = width - (int)strlen(title) - 2;
    const int seg = width = (int)(l / 2);
    assert(l >= 0);

    while (width--) putBoxChar(border.H);
    putchar(' ');
    if (color) putsn(ANSI_COLOR_RESET);
    putsn(title);
    if (color) putsn(ANSI_COLORS[color]);
    putchar(' ');
    width = seg + (l & 1);
  }
  while (width--) putBoxChar(border.H);
  putBoxChar(border.DL);
  putchar('\n');
}

static inline void
drawBoxPadding(unsigned width, byte padding, enum BorderStyle border_style, enum Color color)
{
  while (padding--)
  {
    putBoxChar(border.V);
    if (color) putsn(ANSI_COLOR_RESET);
    unsigned w = width;
    while (w--) putchar(' ');
    if (color) putsn(ANSI_COLORS[color]);
    putBoxChar(border.V);
    putchar('\n');
  }
}

static inline void
drawBoxBottom(unsigned width, enum BorderStyle border_style, enum Color color)
{
  putBoxChar(border.UR);
  while (width--) putBoxChar(border.H);
  putBoxChar(border.UL);
  if (color) putsn(ANSI_COLOR_RESET);
  putchar('\n');
}

void
drawBoxWL(FormattedLines lines, unsigned width, enum BorderStyle border_style, BoxOptions* options)
{
  if (!options) options = &DEFAULT_BoxOptions;
  width -= 2;

  drawBoxTitle(width, options->title, border_style, options->color);
  drawBoxPadding(width, options->paddingY, border_style, options->color);

  FormattedLinesIterator it = formatted_lines_it(lines, 0);
  FormattedLine* line = formatted_lines_it_cur(&it);
  do {
    putBoxChar(border.V);
    if (options->color) putsn(ANSI_COLOR_RESET);
    int p;
    switch (line->kind)
    {
    case WRAPLINEKIND_NONE:
      p = options->paddingX;
      while (p--) putchar(' ');
      putsn(line->text);
      p = options->paddingX;
      while (p--) putchar(' ');
      break;

    case WRAPLINEKIND_CENTER:;
      const int space = width - line->display_length;
      p = space / 2;
      while (p--) putchar(' ');
      putsn(line->text);
      p = space / 2 + (space & 1);
      while (p--) putchar(' ');
      break;

    case WRAPLINEKIND_RTL:
      p = width - options->paddingX - line->display_length;
      while (p--) putchar(' ');
      putsn(line->text);
      p = options->paddingX;
      while (p--) putchar(' ');
      break;

    case WRAPLINEKIND_LTR:
    default:
      p = options->paddingX;
      while (p--) putchar(' ');
      putsn(line->text);
      p = width - options->paddingX - line->display_length;
      while (p--) putchar(' ');
      break;
    }

    if (options->color) putsn(ANSI_COLORS[options->color]);
    putBoxChar(border.V);
    putchar('\n');
    fflush(stdout);
  } while ((line = formatted_lines_it_next(&it)));

  byte py = options->paddingY;
  if (options->height)
  {
    unsigned cur_height = formatted_lines_size(lines) + (options->paddingY + 1) * 2;
    assert(cur_height <= options->height);
    py += options->height - cur_height;
  }
  drawBoxPadding(width, py, border_style, options->color);
  drawBoxBottom(width, border_style, options->color);

  fflush(stdout);
  if (!options->do_not_free) formatted_lines_free(lines);
  if (options->captures)
    while (options->captures_count--)
    {
      options->captures[options->captures_count].x += options->paddingX + 1;
      options->captures[options->captures_count].y += options->paddingY + 1;
    }
}

static void
drawEmptyBox(byte width, byte height, const char title[], enum BorderStyle border_style, enum Color color)
{
  width -= 2;

  drawBoxTitle(width, title, border_style, color);
  drawBoxPadding(width, height - 2, border_style, color);
  drawBoxBottom(width, border_style, color);
}

#undef border

void
showInfoDialog(const char title[], const char text[])
{
  struct BoxOptions options
      = { .title = title, .color = COLOR_YELLOW, .paddingX = DIALOG_PADDING_X, .paddingY = DIALOG_PADDING_Y };
  FormattedLines lines = wrapBox(text, DIALOG_WIDTH, &options);
  lines = addNewline(lines);
  lines = addLine(lines, PRESS_SPACE, WRAPLINEKIND_CENTER);

  putsn(ANSI_CURSOR_HIDE);
  clearStdout();
  drawBoxWL(lines, DIALOG_WIDTH, BORDER_SINGLE, &options);

  fflush(stdout);

  waitForKey(' ');
  putsn(ANSI_CURSOR_SHOW);
}

void
showErrorDialog(const char context[], const char error_text[])
{
  char* text = malloc(strlen(context) + strlen(error_text) + 4);
  sprintf(text, "%s: %s", context, error_text);

  struct BoxOptions options
      = { .title = "ERROR", .color = COLOR_RED, .paddingX = DIALOG_PADDING_X, .paddingY = DIALOG_PADDING_Y };
  FormattedLines lines = wrapBox(text, DIALOG_WIDTH, &options);
  lines = addNewline(lines);
  lines = addLine(lines, PRESS_SPACE, WRAPLINEKIND_CENTER);

  putsn(ANSI_CURSOR_HIDE);
  clearStdout();
  drawBoxWL(lines, DIALOG_WIDTH, BORDER_DOUBLE, &options);

  fflush(stdout);

  waitForKey(' ');
  putsn(ANSI_CURSOR_SHOW);
}

static void
drawScrollIndicator(void)
{
  setCursorPos(SCREEN_WIDTH - 2, SCREEN_HEIGHT - 2);
#ifdef TOT_ASCII
  putsn(ANSI_COLOR_GRAY "v" ANSI_COLOR_RESET);
#else
  putsn(ANSI_COLOR_GRAY "\u2193" ANSI_COLOR_RESET);
#endif
}

static void
removeScrollIndicator(void)
{
  setCursorPos(SCREEN_WIDTH - 2, SCREEN_HEIGHT - 2);
  putchar(' ');
}

void
showLongInfoDialog(const char title[], const char text[], enum Color border_color, bool must_read)
{
  FormattedLines lines = wrapText(text, DIALOG_WIDTH - 4, NULL);

  lines = addNewline(lines);
  lines = addLine(lines, PRESS_SPACE, WRAPLINEKIND_CENTER);

  putsn(ANSI_CURSOR_HIDE);
  clearStdout();

  FormattedLinesIterator it = formatted_lines_it(lines, 0);
  drawEmptyBox(DIALOG_WIDTH, min(SCREEN_HEIGHT, (byte)formatted_lines_size(lines) + 2), title, BORDER_SINGLE, border_color);
  putBlockWLFill(it, min(SCREEN_HEIGHT - 2, (byte)formatted_lines_size(lines)), 2, 1, SCREEN_WIDTH - 3);

  int max_scroll = max((int)formatted_lines_size(lines) - SCREEN_HEIGHT + 2, 0);

  if (max_scroll) drawScrollIndicator();
  fflush(stdout);

  int scroll = 0;
  int key;
  while ((key = getKeyInput()))
  {
    if (key < 0) break;
    if (key == ' ' && (!must_read || scroll >= max_scroll - 3)) break;
    switch (key)
    {
    case KEY_ARROW_DOWN:
      if (scroll < max_scroll)
      {
        ++scroll;
        break;
      }
      continue;

    case KEY_ARROW_UP:
      if (scroll)
      {
        --scroll;
        break;
      }
      continue;

    case KEY_PAGE_UP:
      if (scroll)
      {
        scroll -= SCREEN_HEIGHT - 3;
        if (scroll < 0) scroll = 0;
        break;
      }
      continue;

    case KEY_PAGE_DOWN:
      if (scroll < max_scroll)
      {
        scroll += SCREEN_HEIGHT - 3;
        if (scroll > max_scroll) scroll = max_scroll;
        break;
      }
      continue;

    default: continue;
    }

    // update
    formatted_lines_it_seek(&it, scroll);
    putBlockWLFill(it, SCREEN_HEIGHT - 2, 2, 1, SCREEN_WIDTH - 3);
    if (scroll < max_scroll) drawScrollIndicator();
    else removeScrollIndicator();
    fflush(stdout);
  }
  formatted_lines_free(lines);
}

void
showStoryDialog(size_t count, const struct StoryPage pages[])
{
  for (size_t i = 0; i < count; i++)
  {
    showLongInfoDialog(pages[i].title, pages[i].text, pages[i].border_color, true);
    if (HALT) break;
  }
}

bool
showConfirmationDialog(const char* text)
{
  Coord capture = { 0 };
  clearStdout();
  struct BoxOptions options
      = { .color = COLOR_RED, .paddingX = DIALOG_PADDING_X, .paddingY = DIALOG_PADDING_Y, .captures = &capture };
  drawBox(text, DIALOG_WIDTH, BORDER_DOUBLE, &options);
  setCursorPos(capture.x, capture.y);
  fflush(stdout);

  return getBooleanInput(NULL);
}

void
showPromptDialog(const char prompt[], char* buffer, short buffer_size)
{
  clearStdout();
  const size_t len = strlen(prompt);
  VLA(char, text, len + 3);
  memcpy(text, prompt, len);
  strcpy(text + len, " " CAPTURE_STRING);

  Coord capture = { 0 };
  drawBox(text, DIALOG_WIDTH, BORDER_DOUBLE, &(struct BoxOptions){ .color = COLOR_YELLOW, .captures = &capture });
  setCursorPos(capture.x, capture.y);
  putsn(ANSI_CURSOR_SHOW);
  fflush(stdout);

  getStringInput(buffer, 1, buffer_size, NULL);
}

void
putBlock(const char* text, byte x, byte y)
{
  char* _text = strdup(text);
  char* line;
  byte i = 0;

  line = strtok(_text, "\n");
  while (line != 0)
  {
    setCursorPos(x, y + i++);
    putsn(line);
    line = strtok(0, "\n");
  }
  fflush(stdout);
  free(_text);
}

void
putBlockWL(FormattedLines lines, byte x, byte y, byte width)
{
  byte i = 0;
  FormattedLinesIterator it = formatted_lines_it(lines, 0);
  FormattedLine* line = formatted_lines_it_cur(&it);
  do {
    switch (line->kind)
    {
    case WRAPLINEKIND_NONE:
    case WRAPLINEKIND_LTR:
      setCursorPos(x, y + i);
      putsn(line->text);
      break;

    case WRAPLINEKIND_CENTER:;
      const int space = width - line->display_length;
      byte p = space / 2;
      setCursorPos(x + p, y + i);
      putsn(line->text);
      break;

    case WRAPLINEKIND_RTL:
      setCursorPos(x + width - line->display_length, y + i);
      putsn(line->text);
      break;
    }
  } while (i++, line = formatted_lines_it_next(&it));

  formatted_lines_free(lines);
}

void
putBlockWLFill(FormattedLinesIterator it, unsigned count, byte x, byte y, byte width)
{
  assert(count <= formatted_lines_size(it.lines));
  FormattedLine* line = formatted_lines_it_cur(&it);
  for (byte i = 0; i < count; i++, line = formatted_lines_it_next(&it))
  {
    setCursorPos(x, y + i);
    byte p;

    switch (line->kind)
    {
    case WRAPLINEKIND_NONE: putsn(line->text); break;

    case WRAPLINEKIND_LTR:
      putsn(line->text);
      p = width - line->display_length;
      while (p--) putchar(' ');
      break;

    case WRAPLINEKIND_CENTER:;
      const int space = width - line->display_length;
      p = space / 2;
      while (p--) putchar(' ');
      putsn(line->text);
      p = space / 2 + (space & 1);
      while (p--) putchar(' ');
      break;

    case WRAPLINEKIND_RTL:
      p = width - line->display_length;
      while (p--) putchar(' ');
      putsn(line->text);
      break;
    }
  }
}

CHECK_RETURN FormattedLines
addBar(FormattedLines lines, byte length, char c, enum Color color)
{
  VLA(char, buf, length + 16); // allocate room for color codes

  char* end = buf;
  if (color)
  {
    strcpy(buf, ANSI_COLORS[color]);
    end += strlen(ANSI_COLORS[color]);
  }
  memset(end, c, length);
  end[length] = '\0';
  if (color)
  {
    strcpy(end + length, ANSI_COLOR_RESET);
  }
  return addLine(lines, buf, WRAPLINEKIND_LTR);
}

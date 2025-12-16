#include <assert.h>
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
  .height = 0,
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

static enum QKeyCallbackReturn
inputCallback(int key, va_list args)
{
  int* cur_pos = va_arg(args, int*);
  const int choices_size = va_arg(args, const int);
  const struct ChoiceDialogChoice* choices = va_arg(args, const struct ChoiceDialogChoice*);
  const ChoiceDialogCallback callback = va_arg(args, const ChoiceDialogCallback);
  const FormattedLines lines = va_arg(args, const FormattedLines);
  const struct ChoiceInfo* choices_info = va_arg(args, const struct ChoiceInfo*);
  const Coord offset = va_arg(args, const Coord);
  const Coord end = va_arg(args, const Coord);

  switch (key)
  {
  case ETR_CHAR:
    if (*cur_pos != -1)
    {
      putsn(ANSI_CURSOR_SHOW);
      const struct ChoiceDialogChoice* choice = &choices[*cur_pos];
      if (choice->callback) (*choice->callback)(choice);
      else if (callback) (*callback)(choice, *cur_pos);
      return QKEY_CALLBACK_RETURN_END;
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

  return *cur_pos == -1 ? QKEY_CALLBACK_RETURN_NORMAL : QKEY_CALLBACK_RETURN_IGNORE;
}

void
showChoiceDialog(const char* text, unsigned choices_size, const struct ChoiceDialogChoice choices[], DialogOptions* options)
{
  FormattedLines lines = wrapText(text, DIALOG_CONTENT_WIDTH, NULL);
  lines = addNewline(lines);
  showChoiceDialogWL(lines, choices_size, choices, options);
}

void
showChoiceDialogWL(
    FormattedLines lines, unsigned choices_size, const struct ChoiceDialogChoice choices[], DialogOptions* options
)
{
  if (!options)
  {
    options = &DEFAULT_DialogOptions;
  }
  else if (!options->color)
  {
    options->color = DEFAULT_DialogOptions.color;
  }
  if (!lines)
  {
    lines = formatted_lines_create(choices_size);
  }
  const byte padding_y = options->noPaddingY ? 0 : DIALOG_PADDING_Y;
  Coord capture = { 0 };
  struct ChoiceInfo* choices_info = (struct ChoiceInfo*)malloc(sizeof(struct ChoiceInfo) * choices_size);
  assert(choices_info);

  for (unsigned i = 0; i < choices_size; i++)
  {
    byte added_count = 0;
    char str[64];
    sprintf(str, "%i. %s", i + 1, choices[i].name);
    choices_info[i].start = (byte)formatted_lines_size(lines);
    lines = wrapText(str, DIALOG_CONTENT_WIDTH, &(struct WrapLineOptions){ .lines = lines, .added_count = &added_count });
    choices_info[i].end = (byte)choices_info[i].start + added_count;
  }
  lines = addNewline(lines);

  char str2[] = "What is your choice? " CAPTURE_STRING;
  lines = wrapText(str2, DIALOG_CONTENT_WIDTH, &(struct WrapLineOptions){ .lines = lines, .captures = &capture });
  capture.x += 1 + DIALOG_PADDING_X;
  capture.y += 1 + padding_y;

  clearStdout();
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
                      1, choices_size, 0, &inputCallback, &cur_pos, choices_size, choices, options->callback, lines,
                      choices_info, offset, capture
                  )
                  - 1;

  ChoiceDialogCallback options_callback = options->callback;
  formatted_lines_free(lines);
  free(choices_info);

  if (num >= 0)
  {
    const struct ChoiceDialogChoice* choice = &choices[num];
    if (choice->callback) (*choice->callback)(choice);
    else if (options_callback) (*options_callback)(choice, num);
  }
}

void
drawChoice(
    const struct ChoiceDialogChoice* choices, const FormattedLines lines, const struct ChoiceInfo* choices_info, Coord offset,
    int index, bool selected
)
{
  (void)choices;

  escape_combo = 0;
  if (selected) putsn(ANSI_SELECTED);
  for (byte i = choices_info[index].start; i < choices_info[index].end; i++)
  {
    setCursorPos(offset.x, offset.y + i);
    putsn(formatted_lines_at(lines, i)->text);
  }
  if (selected) putsn(ANSI_COLOR_RESET);
  fflush(stdout);
}

FormattedLines
wrapBox(const char* text, unsigned width, BoxOptions* options)
{
  struct WrapLineOptions wrap_options = { .captures = options->captures };
  FormattedLines lines = wrapText(text, width - options->paddingX * 2, &wrap_options);
  options->captures_count = wrap_options.captures_count;
  return lines;
}

void
drawBox(const char* text, unsigned width, enum BorderStyle border, BoxOptions* options)
{
  drawBoxWL(wrapBox(text, width, options), width, border, options);
}

#define border BOX_CHAR_BORDERS[border_style]

void
drawBoxWL(FormattedLines lines, unsigned width, enum BorderStyle border_style, BoxOptions* options)
{
  // TODO: options->height
  if (!options)
  {
    options = &DEFAULT_BoxOptions;
  }
  width -= 2;
  if (options->title) assert(*options->title && strlen(options->title) <= width);
  int w = width;

  if (options->color) putsn(ANSI_COLORS[options->color]);
  putBoxChar(border.DR);
  if (options->title)
  {
    const int l = width - (int)strlen(options->title) - 2;
    const int seg = w = (int)(l / 2);
    while (w--) putBoxChar(border.H);
    putchar(' ');
    if (options->color) putsn(ANSI_COLOR_RESET);
    putsn(options->title);
    if (options->color) putsn(ANSI_COLORS[options->color]);
    putchar(' ');
    w = seg;
    if (l % 2) w++;
    while (w--) putBoxChar(border.H);
  }
  else
    while (w--) putBoxChar(border.H);
  putBoxChar(border.DL);
  putchar('\n');

  byte py = options->paddingY;
  while (py--)
  {
    putBoxChar(border.V);
    if (options->color) putsn(ANSI_COLOR_RESET);
    w = width;
    while (w--) putchar(' ');
    if (options->color) putsn(ANSI_COLORS[options->color]);
    putBoxChar(border.V);
    putchar('\n');
  }

  for (byte l = 0; l < formatted_lines_size(lines); l++)
  {
    putBoxChar(border.V);
    if (options->color) putsn(ANSI_COLOR_RESET);
    int p;
    FormattedLine* line = formatted_lines_at(lines, l);
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
  }

  py = options->paddingY;
  while (py--)
  {
    putBoxChar(border.V);
    if (options->color) putsn(ANSI_COLOR_RESET);
    w = width;
    while (w--) putchar(' ');
    if (options->color) putsn(ANSI_COLORS[options->color]);
    putBoxChar(border.V);
    putchar('\n');
  }

  putBoxChar(border.UR);
  w = width;
  while (w--) putBoxChar(border.H);
  putBoxChar(border.UL);
  if (options->color) putsn(ANSI_COLOR_RESET);
  putchar('\n');

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
  int w = width;

  if (color) putsn(ANSI_COLORS[color]);
  putBoxChar(border.DR);
  if (title)
  {
    const int l = width - (int)strlen(title) - 2;
    const int seg = w = (int)(l / 2);
    while (w--) putBoxChar(border.H);
    putchar(' ');
    if (color) putsn(ANSI_COLOR_RESET);
    putsn(title);
    if (color) putsn(ANSI_COLORS[color]);
    putchar(' ');
    w = seg;
    if (l % 2) w++;
    while (w--) putBoxChar(border.H);
  }
  else
    while (w--) putBoxChar(border.H);
  putBoxChar(border.DL);
  putchar('\n');

  while (height-- > 2)
  {
    putBoxChar(border.V);
    if (color) putsn(ANSI_COLOR_RESET);
    w = width;
    while (w--) putchar(' ');
    if (color) putsn(ANSI_COLORS[color]);
    putBoxChar(border.V);
    putchar('\n');
  }

  putBoxChar(border.UR);
  w = width;
  while (w--) putBoxChar(border.H);
  putBoxChar(border.UL);
  if (color) putsn(ANSI_COLOR_RESET);
}

#undef border

void
showInfoDialog(const char title[], const char text[])
{
  struct BoxOptions options
      = { .title = title, .color = COLOR_YELLOW, .paddingX = DIALOG_PADDING_X, .paddingY = DIALOG_PADDING_Y };
  FormattedLines lines = wrapBox(text, DIALOG_WIDTH, &options);
  lines = addNewline(lines);

  FormattedLine line = {
    .length = sizeof(PRESS_SPACE) - 1,
    .display_length = (byte)_strlen_iae(PRESS_SPACE),
    .kind = WRAPLINEKIND_CENTER,
  };
  lines = formatted_lines_add(lines, line, PRESS_SPACE);

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

  struct FormattedLine line = {
    .length = sizeof(PRESS_SPACE) - 1,
    .display_length = (byte)_strlen_iae(PRESS_SPACE),
    .kind = WRAPLINEKIND_CENTER,
  };
  lines = formatted_lines_add(lines, line, PRESS_SPACE);

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
showLongInfoDialog(const char title[], const char text[], enum Color border_color)
{
  FormattedLines lines = wrapText(text, DIALOG_WIDTH - 3, NULL);

  lines = addNewline(lines);
  lines = addLine(lines, PRESS_SPACE, WRAPLINEKIND_CENTER);

  putsn(ANSI_CURSOR_HIDE);
  clearStdout();

  drawEmptyBox(DIALOG_WIDTH, min(SCREEN_HEIGHT, (byte)formatted_lines_size(lines) + 2), title, BORDER_SINGLE, border_color);
  putBlockWLFill(lines, 0, min(SCREEN_HEIGHT - 2, (byte)formatted_lines_size(lines)), 2, 1, SCREEN_WIDTH - 3);

  int max_scroll = max((int)formatted_lines_size(lines) - SCREEN_HEIGHT + 2, 0);

  if (max_scroll) drawScrollIndicator();
  fflush(stdout);

  int scroll = 0;
  int key;
  while ((key = getKeyInput()))
  {
    if (key < 0) break;
    if (key == ' ' && scroll >= max_scroll - 3) break;
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
    putBlockWLFill(lines, scroll, scroll + SCREEN_HEIGHT - 2, 2, 1, SCREEN_WIDTH - 3);
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
#ifndef TOT_MUTE
    if (pages[i].music_path)
    {
    }
#endif
    showLongInfoDialog(pages[i].title, pages[i].text, pages[i].border_color);
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
showPromptDialog(const char text[], char* buffer, short buffer_size)
{
  clearStdout();
  const size_t len = strlen(text);
  char* _text = malloc(len + 3);
  assert(_text);
  memcpy(_text, text, len);
  _text[len] = ' ';
  _text[len + 1] = CAPTURE_CHAR;
  _text[len + 2] = 0;

  Coord capture = { 0 };
  drawBox(_text, DIALOG_WIDTH, BORDER_DOUBLE, &(struct BoxOptions){ .color = COLOR_YELLOW, .captures = &capture });
  setCursorPos(capture.x, capture.y);
  putsn(ANSI_CURSOR_SHOW);
  fflush(stdout);
  free(_text);

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
  for (byte i = 0; i < (byte)formatted_lines_size(lines); i++)
  {
    FormattedLine* line = formatted_lines_at(lines, i);
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
  }

  formatted_lines_free(lines);
}

void
putBlockWLFill(const FormattedLines lines, byte begin_i, byte end_i, byte x, byte y, byte width)
{
  for (byte i = 0; i < end_i - begin_i; i++)
  {
    FormattedLine* line = formatted_lines_at(lines, begin_i + i);
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

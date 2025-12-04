#include "cvector.h"
#include <assert.h>
#include <ctype.h>
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
    const struct ChoiceDialogChoice* choices, const struct WrapLine* lines, const struct ChoiceInfo* choices_info, Coord offset,
    int index, bool selected
);

byte SCREEN_WIDTH;
byte SCREEN_HEIGHT;
byte DIALOG_CONTENT_WIDTH;

static const char press_space[] = ANSI_COLOR_GREEN "Press SPACE BAR to continue" ANSI_COLOR_RESET;

struct WrapLine*
wrapText(const char* text, int width, WrapLineOptions options)
{
  bool free_options = 0;
  if (!options)
  {
    free_options = 1;
    options = malloc(sizeof(struct WrapLineOptions));
    assert(options);
    options->captures = NULL;
    options->height = 0;
    options->kind = WRAPLINEKIND_LTR;
    options->lines = NULL;
    options->added_count = NULL;
  }

  struct WrapLine* lines = NULL;
  if (options->lines)
  {
    lines = options->lines;
  }
  else
  {
    cvector_init(lines, 0, 0);
  }
  byte start = (byte)cvector_size(lines);
  cvector_push_back_struct(lines);
  assert(lines);
  byte l = start;
  lines[l].length = 0;
  lines[l].kind = options->kind;

  const bool auto_height = options->height; // todo

  int i = -1;
  bool is_escaping = 0;
  byte escaped_chars = 0;
  int last_break = 0;
  int capture_i = 0;

  while (text[++i])
  {
    if (text[i] == '\n')
    {
    newline:
      lines[l].client_length = lines[l].length - escaped_chars;
      lines[l].text[lines[l].length++] = 0;
      ++l;
      cvector_push_back_struct(lines);
      lines[l].length = 0;
      lines[l].kind = options->kind;
      lines[l].text[0] = 0;
      escaped_chars = 0;
      continue;
    }
    else if (text[i] == ESC_CHAR) is_escaping = 1;
    else if (text[i] == ' ') last_break = i;
    else if (text[i] == CONTROL_CHAR && options->captures)
    {
      options->captures[capture_i].y = l;
      options->captures[capture_i++].x = lines[l].length;
      continue;
    }

    if (is_escaping)
    {
      ++escaped_chars;
      if (isalpha(text[i])) is_escaping = 0;
    }
    lines[l].text[lines[l].length++] = text[i];
    if (lines[l].length - escaped_chars == width)
    {
      lines[l].text[lines[l].length -= i - last_break + 1] = 0;
      i = last_break;
      goto newline;
    }
  }
  lines[l].client_length = lines[l].length - escaped_chars;
  lines[l].text[lines[l].length++] = 0;

  if (options->added_count) *options->added_count = l - start + (byte)1;
  options->captures_count = capture_i;
  if (free_options) free(options);
  return lines;
}

struct WrapLine*
addNewline(struct WrapLine* lines)
{
  cvector_push_back_struct(lines);
  struct WrapLine* last_line = &cvector_last(lines);
  last_line->length = 0;
  last_line->client_length = 0;
  last_line->text[0] = 0;
  last_line->kind = WRAPLINEKIND_LTR;
  return lines;
}

struct WrapLine*
addLine(struct WrapLine* lines, const char* text, WrapLineKind kind)
{
  cvector_push_back_struct(lines);
  struct WrapLine* last_line = &cvector_last(lines);
  last_line->length = (byte)strlen(text);
  last_line->client_length = (byte)_strlen_iae(text);
  last_line->kind = kind;
  if (text) memcpy(last_line->text, text, (size_t)last_line->length + 1);
  return lines;
}

struct WrapLine*
justifyLineWL(struct WrapLine* lines, const char* text1, const char* text2, const byte width)
{
  cvector_push_back_struct(lines);
  struct WrapLine* last_line = &cvector_last(lines);
  last_line->length = width;
  last_line->client_length = width;
  last_line->kind = WRAPLINEKIND_NONE;
  const byte len2 = (byte)strlen(text2);
  byte pos = (byte)strlen(text1);
  memcpy(last_line->text, text1, pos);
  while (pos + len2 < width) last_line->text[pos++] = ' ';
  memcpy(last_line->text + pos, text2, (size_t)len2);
  pos += len2;
  while (pos < width) last_line->text[pos++] = ' ';
  last_line->text[pos] = 0;
  return lines;
}

static enum QKeyCallbackReturn
inputCallback(int key, va_list args)
{
  int* cur_pos = va_arg(args, int*);
  const int choices_size = va_arg(args, const int);
  const struct ChoiceDialogChoice* choices = va_arg(args, const struct ChoiceDialogChoice*);
  const ChoiceDialogCallback callback = va_arg(args, const ChoiceDialogCallback);
  const struct WrapLine* lines = va_arg(args, const struct WrapLine*);
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
showChoiceDialog(
    const char* text, int choices_size, const struct ChoiceDialogChoice choices[choices_size], DialogOptions options
)
{
  struct WrapLine* lines = wrapText(text, DIALOG_CONTENT_WIDTH, NULL);
  lines = addNewline(lines);
  showChoiceDialogWL(lines, choices_size, choices, options);
}

void
showChoiceDialogWL(
    struct WrapLine* lines, int choices_size, const struct ChoiceDialogChoice choices[choices_size], DialogOptions options
)
{
  bool free_options = 0;
  if (!options)
  {
    free_options = 1;
    options = malloc(sizeof(struct DialogOptions));
    options->title = NULL;
    options->noPaddingY = 0;
    options->callback = NULL;
    options->color = COLOR_YELLOW;
  }
  else
  {
    if (!options->color) options->color = COLOR_YELLOW;
  }
  const byte padding_y = options->noPaddingY ? 0 : DIALOG_PADDING_Y;
  Coord capture = { 0 };
  struct ChoiceInfo* choices_info = (struct ChoiceInfo*)malloc(sizeof(struct ChoiceInfo) * choices_size);
  assert(choices_info);

  for (int i = 0; i < choices_size; i++)
  {
    byte added_count = 0;
    char str[64] = { 0, '.', ' ', 0 };
    str[0] = '1' + i;
    strcat(str, choices[i].name);
    choices_info[i].start = (byte)cvector_size(lines);
    lines = wrapText(str, DIALOG_CONTENT_WIDTH, &(struct WrapLineOptions){ .lines = lines, .added_count = &added_count });
    choices_info[i].end = (byte)choices_info[i].start + added_count;
  }
  lines = addNewline(lines);

  char str2[] = "What is your choice? " CONTROL_CHAR_STR;
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

  cvector_free(lines);
  free(choices_info);

  if (num >= 0)
  {
    const struct ChoiceDialogChoice* choice = &choices[num];
    if (choice->callback) (*choice->callback)(choice);
    else if (options->callback) (*options->callback)(choice, num);
  }
  if (free_options) free(options);
}

void
drawChoice(
    const struct ChoiceDialogChoice* choices, const struct WrapLine* lines, const struct ChoiceInfo* choices_info, Coord offset,
    int index, bool selected
)
{
  escape_combo = 0;
  if (selected) putsn(ANSI_SELECTED);
  for (byte i = choices_info[index].start; i < choices_info[index].end; i++)
  {
    setCursorPos(offset.x, offset.y + i);
    putsn(lines[i].text);
  }
  if (selected) putsn(ANSI_COLOR_RESET);
  fflush(stdout);
}

struct WrapLine*
wrapBox(const char* text, int width, const BoxOptions options)
{
  struct WrapLineOptions wrap_options = { .height = options->height - options->paddingY * 2, .captures = options->captures };
  struct WrapLine* lines = wrapText(text, width - options->paddingX * 2, &wrap_options);
  options->captures_count = wrap_options.captures_count;
  return lines;
}

void
drawBox(char* text, int width, enum BorderStyle border, BoxOptions options)
{
  drawBoxWL(wrapBox(text, width, options), width, border, options);
}

void
drawBoxWL(struct WrapLine* lines, int width, enum BorderStyle border, BoxOptions options)
{
  bool free_options = 0;
  if (!options)
  {
    free_options = 1;
    options = malloc(sizeof(struct BoxOptions));
    options->captures = NULL;
    options->color = COLOR_DEFAULT;
    options->do_not_free = 0;
    options->height = 0;
    options->paddingX = DIALOG_PADDING_X;
    options->paddingY = DIALOG_PADDING_Y;
    options->title = NULL;
  }
  width -= 2;
  if (options->title) assert(*options->title && strlen(options->title) <= width);
  int w = width;

  // clang-format off
  const box_char_t
      BORDER_V = border ? BOX_CHAR_D_V : BOX_CHAR_V,
      BORDER_H = border ? BOX_CHAR_D_H : BOX_CHAR_H,
      BORDER_DR = border ? BOX_CHAR_D_DR : BOX_CHAR_DR,
      BORDER_DL = border ? BOX_CHAR_D_DL : BOX_CHAR_DL,
      BORDER_UR = border ? BOX_CHAR_D_UR : BOX_CHAR_UR,
      BORDER_UL = border ? BOX_CHAR_D_UL : BOX_CHAR_UL;
  // clang-format on

  if (options->color) putsn(ANSI_COLORS[options->color]);
  putBoxChar(BORDER_DR);
  if (options->title)
  {
    const int l = width - (int)strlen(options->title) - 2;
    const int seg = w = (int)(l / 2);
    while (w--) putBoxChar(BORDER_H);
    putchar(' ');
    if (options->color) putsn(ANSI_COLOR_RESET);
    putsn(options->title);
    if (options->color) putsn(ANSI_COLORS[options->color]);
    putchar(' ');
    w = seg;
    if (l % 2) w++;
    while (w--) putBoxChar(BORDER_H);
  }
  else
    while (w--) putBoxChar(BORDER_H);
  putBoxChar(BORDER_DL);
  putchar('\n');

  byte py = options->paddingY;
  while (py--)
  {
    putBoxChar(BORDER_V);
    if (options->color) putsn(ANSI_COLOR_RESET);
    w = width;
    while (w--) putchar(' ');
    if (options->color) putsn(ANSI_COLORS[options->color]);
    putBoxChar(BORDER_V);
    putchar('\n');
  }

  for (byte l = 0; l < cvector_size(lines); l++)
  {
    putBoxChar(BORDER_V);
    if (options->color) putsn(ANSI_COLOR_RESET);
    int p;
    switch (lines[l].kind)
    {
    case WRAPLINEKIND_NONE:
      p = options->paddingX;
      while (p--) putchar(' ');
      putsn(lines[l].text);
      p = options->paddingX;
      while (p--) putchar(' ');
      break;

    case WRAPLINEKIND_CENTER:;
      const int space = width - lines[l].client_length;
      p = space / 2;
      while (p--) putchar(' ');
      putsn(lines[l].text);
      p = space / 2 + (space & 1);
      while (p--) putchar(' ');
      break;

    case WRAPLINEKIND_RTL:
      p = width - options->paddingX - lines[l].client_length;
      while (p--) putchar(' ');
      putsn(lines[l].text);
      p = options->paddingX;
      while (p--) putchar(' ');
      break;

    case WRAPLINEKIND_LTR:
    default:
      p = options->paddingX;
      while (p--) putchar(' ');
      putsn(lines[l].text);
      p = width - options->paddingX - lines[l].client_length;
      while (p--) putchar(' ');
      break;
    }

    if (options->color) putsn(ANSI_COLORS[options->color]);
    putBoxChar(BORDER_V);
    putchar('\n');
    fflush(stdout);
  }

  py = options->paddingY;
  while (py--)
  {
    putBoxChar(BORDER_V);
    if (options->color) putsn(ANSI_COLOR_RESET);
    w = width;
    while (w--) putchar(' ');
    if (options->color) putsn(ANSI_COLORS[options->color]);
    putBoxChar(BORDER_V);
    putchar('\n');
  }

  putBoxChar(BORDER_UR);
  w = width;
  while (w--) putBoxChar(BORDER_H);
  putBoxChar(BORDER_UL);
  if (options->color) putsn(ANSI_COLOR_RESET);
  putchar('\n');

  fflush(stdout);
  if (!options->do_not_free) cvector_free(lines);
  if (options->captures)
    while (options->captures_count--)
    {
      options->captures[options->captures_count].x += options->paddingX + 1;
      options->captures[options->captures_count].y += options->paddingY + 1;
    }
  if (free_options) free(options);
}

void
showInfoDialog(const char title[], const char text[])
{
  struct BoxOptions options
      = { .title = title, .color = COLOR_YELLOW, .paddingX = DIALOG_PADDING_X, .paddingY = DIALOG_PADDING_Y };
  struct WrapLine* lines = wrapBox(text, DIALOG_WIDTH, &options);
  lines = addNewline(lines);

  cvector_push_back_struct(lines);
  struct WrapLine* last_line = &cvector_last(lines);

  memcpy(last_line->text, press_space, sizeof(press_space));
  last_line->length = sizeof(press_space) - 1;
  last_line->client_length = (byte)_strlen_iae(press_space);
  last_line->kind = WRAPLINEKIND_CENTER;

  putsn(ANSI_CURSOR_HIDE);
  clearStdout();
  drawBoxWL(lines, DIALOG_WIDTH, BORDER_SINGLE, &options);

  fflush(stdout);

  waitForKey(' ');
  putsn(ANSI_CURSOR_SHOW);
}

static void
drawEmptyBox(byte width, byte height, enum BorderStyle border, enum Color color)
{
  width -= 2;
  int w = width;

  // clang-format off
  const box_char_t
      BORDER_V = border ? BOX_CHAR_D_V : BOX_CHAR_V,
      BORDER_H = border ? BOX_CHAR_D_H : BOX_CHAR_H,
      BORDER_DR = border ? BOX_CHAR_D_DR : BOX_CHAR_DR,
      BORDER_DL = border ? BOX_CHAR_D_DL : BOX_CHAR_DL,
      BORDER_UR = border ? BOX_CHAR_D_UR : BOX_CHAR_UR,
      BORDER_UL = border ? BOX_CHAR_D_UL : BOX_CHAR_UL;
  // clang-format on

  if (color) putsn(ANSI_COLORS[color]);
  putBoxChar(BORDER_DR);
  while (w--) putBoxChar(BORDER_H);
  putBoxChar(BORDER_DL);
  putchar('\n');

  while (height-- > 2)
  {
    putBoxChar(BORDER_V);
    if (color) putsn(ANSI_COLOR_RESET);
    w = width;
    while (w--) putchar(' ');
    if (color) putsn(ANSI_COLORS[color]);
    putBoxChar(BORDER_V);
    putchar('\n');
  }

  putBoxChar(BORDER_UR);
  w = width;
  while (w--) putBoxChar(BORDER_H);
  putBoxChar(BORDER_UL);
  if (color) putsn(ANSI_COLOR_RESET);
  putchar('\n');
}

static void
drawScrollIndicator(void)
{
  setCursorPos(SCREEN_WIDTH, SCREEN_HEIGHT - 2);
#ifdef TOT_ASCII
  putsn(ANSI_COLOR_GRAY "v" ANSI_COLOR_RESET);
#else
  putsn(ANSI_COLOR_GRAY "\u2193" ANSI_COLOR_RESET);
#endif
}

static void
removeScrollIndicator(void)
{
  setCursorPos(SCREEN_WIDTH, SCREEN_HEIGHT - 2);
  putchar(' ');
}

void
showLongInfoDialog(const char title[], const char text[], enum Color border_color)
{
  struct WrapLine* lines = wrapText(text, SCREEN_WIDTH - 3, NULL);

  lines = addNewline(lines);
  lines = addLine(lines, press_space, WRAPLINEKIND_CENTER);

  putsn(ANSI_CURSOR_HIDE);
  clearStdout();

  drawEmptyBox(SCREEN_WIDTH, (byte)cvector_size(lines) + 2, BORDER_SINGLE, border_color);
  putBlockWLFill((byte)cvector_size(lines), lines, 2, 1, SCREEN_WIDTH - 3);

  int max_scroll = max((int)cvector_size(lines) - SCREEN_HEIGHT + 1, 0);

  if (max_scroll) drawScrollIndicator();
  fflush(stdout);

  int scroll = 0;
  int key;
  while ((key = getKeyInput()))
  {
    if (key < 0) break;
    if (key == ' ' && scroll >= max_scroll - 2) break;
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
    putBlockWLFill(SCREEN_HEIGHT - 1, lines + scroll, 2, 1, SCREEN_WIDTH - 2);
    if (scroll < max_scroll) drawScrollIndicator();
    else removeScrollIndicator();
    fflush(stdout);
  }
  cvector_free(lines);
}

void
showStoryDialog(size_t count, const struct StoryPage pages[count])
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
showConfirmationDialog(char* text)
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
  _text[len + 1] = CONTROL_CHAR;
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
showErrorDialog(const char* context, int error_code)
{
  char buffer[256];
  if (error_code)
  {
    snprintf(buffer, sizeof(buffer), "%s: %i", context, error_code);
    buffer[sizeof(buffer) - 1] = 0;
  }
  else
  {
    strcpy(buffer, context);
  }
  showInfoDialog("! Error !", buffer);
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
putBlockWL(struct WrapLine* lines, byte x, byte y, byte width)
{
  for (byte i = 0; i < (byte)cvector_size(lines); i++)
  {
    switch (lines[i].kind)
    {
    case WRAPLINEKIND_NONE:
    case WRAPLINEKIND_LTR:
      setCursorPos(x, y + i);
      putsn(lines[i].text);
      break;

    case WRAPLINEKIND_CENTER:;
      const int space = width - lines[i].client_length;
      byte p = space / 2;
      setCursorPos(x + p, y + i);
      putsn(lines[i].text);
      break;

    case WRAPLINEKIND_RTL:
      setCursorPos(x + width - lines[i].client_length, y + i);
      putsn(lines[i].text);
      break;
    }
  }

  cvector_free(lines);
}

void
putBlockWLFill(byte count, struct WrapLine lines[count], byte x, byte y, byte width)
{
  for (byte i = 0; i < count; i++)
  {
    setCursorPos(x, y + i);
    byte p;

    switch (lines[i].kind)
    {
    case WRAPLINEKIND_NONE: putsn(lines[i].text); break;

    case WRAPLINEKIND_LTR:
      putsn(lines[i].text);
      p = width - lines[i].client_length;
      while (p--) putchar(' ');
      break;

    case WRAPLINEKIND_CENTER:;
      const int space = width - lines[i].client_length;
      p = space / 2;
      while (p--) putchar(' ');
      putsn(lines[i].text);
      p = space / 2 + (space & 1);
      while (p--) putchar(' ');
      break;

    case WRAPLINEKIND_RTL:
      p = width - lines[i].client_length;
      while (p--) putchar(' ');
      putsn(lines[i].text);
      break;
    }
  }
}

void
indentLines(struct WrapLine* begin, struct WrapLine* end, const byte amount)
{
  while (begin < end)
  {
    assert(begin->length + amount <= sizeof(begin->text));
    memmove(begin->text + amount, begin->text, begin->length);
    memset(begin->text, ' ', amount);
    begin->length += amount;
    begin->client_length += amount;
    ++begin;
  }
}

struct WrapLine*
textToLines(const char* text)
{
  struct WrapLine* lines = NULL;
  cvector_init(lines, 0, NULL);
  return textToLinesWL(lines, text);
}

struct WrapLine*
textToLinesWL(struct WrapLine* lines, const char* text)
{
  char* _text = strdup(text);
  char* line;

  // split by newline
  line = strtok(_text, "\n");
  while (line)
  {
    cvector_push_back_struct(lines);
    struct WrapLine* last_line = &cvector_last(lines);

    last_line->length = (byte)strlen(line);
    last_line->client_length = last_line->length;
    last_line->kind = WRAPLINEKIND_LTR;
    memcpy(last_line->text, line, last_line->length);
    last_line->text[last_line->length] = 0;
    line = strtok(0, "\n");
  }
  free(_text);
  return lines;
}

struct WrapLine*
addBar(struct WrapLine* lines)
{
  char buf[64] = ANSI_COLOR_CYAN;
  byte l = min(DIALOG_CONTENT_WIDTH, 40);
  memset(buf + sizeof(ANSI_COLOR_CYAN) - 1, '-', l);
  buf[sizeof(ANSI_COLOR_CYAN) + l] = 0;
  return addLine(lines, buf, WRAPLINEKIND_LTR);
}

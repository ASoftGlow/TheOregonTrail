#pragma once
#include <stddef.h>

#include "ansi_codes.h"
#include "formatted_lines.h"

struct ChoiceDialogChoice;

typedef void (*ChoiceCallback)(const struct ChoiceDialogChoice*);
typedef void (*ChoiceDialogCallback)(const struct ChoiceDialogChoice*, const int);

struct ChoiceDialogChoice
{
  const char* name;
  ChoiceCallback callback;
};

enum BorderStyle
{
  BORDER_SINGLE,
  BORDER_DOUBLE,
  __BORDER_END,
};

typedef struct BoxOptions
{
  const char* title;
  int height;
  enum Color color;
  byte paddingX;
  byte paddingY;
  Coord* captures;
  byte captures_count;
  bool do_not_free;
} BoxOptions;

typedef struct DialogOptions
{
  char* title;
  ChoiceDialogCallback callback;
  enum Color color;
  bool noPaddingY;
} DialogOptions;

struct StoryPage
{
  const char* title;
  const char* text;
  const char* music_path;
  const enum Color border_color;
};

/**
 * @brief Wraps <text> and applies a border
 * @param options - is optional
 * @returns a new cvector of lines
 */
FormattedLines wrapBox(const char* text, unsigned width, BoxOptions* options);

/**
 * @brief Wraps <text>, applies a border, and draws it
 * @param border - possible values: BORDER_SINGLE, BORDER_DOUBLE
 * @param options - is optional
 */
void drawBox(const char* text, unsigned width, enum BorderStyle border, BoxOptions* options);

/**
 * @brief Applies a border to <lines> and draws it
 * @param border - possible values: BORDER_SINGLE, BORDER_DOUBLE
 * @param options - is optional
 */
void drawBoxWL(const FormattedLines lines, unsigned width, enum BorderStyle border, BoxOptions* options);

/**
 * @brief Wraps <text>
 * @param options - is optional
 */
void
showChoiceDialog(const char* text, unsigned choices_size, const struct ChoiceDialogChoice choices[], DialogOptions* options);

/**
 * @param options - is optional
 */
void showChoiceDialogWL(
    FormattedLines lines, unsigned choices_size, const struct ChoiceDialogChoice choices[], DialogOptions* options
);
void showInfoDialog(const char title[], const char text[]);
void showLongInfoDialog(const char title[], const char text[], enum Color border_color);
void showStoryDialog(size_t count, const struct StoryPage pages[]);
bool showConfirmationDialog(const char* text);
void showPromptDialog(const char text[], char* buffer, short buffer_size);
void showErrorDialog(const char context[], const char error_text[]);

/**
 * @brief Draws block of text at <x> and <y>
 */
void putBlock(const char* text, byte x, byte y);

/**
 * @brief Draws lines at <x> and <y>
 */
void putBlockWL(const FormattedLines lines, byte x, byte y, byte width);

/**
 * @brief Draws block of lines at <x> and <y> and fills background with whitespace
 */
void putBlockWLFill(const FormattedLines lines, byte begin_i, byte end_i, byte x, byte y, byte width);

// box drawing chars
#ifdef TOT_ASCII
typedef char box_char_t;
#define putBoxChar(ascii) putchar(ascii)
#else
typedef char* box_char_t;
#define putBoxChar(utf8) putsn(utf8)
#endif

/*
 * D = down
 * U = up
 * R = right
 * L = left
 * H = horizontal
 * V = vertical
 */
typedef struct
{
  box_char_t DR, DL, H, V, UR, UL;
} BoxCharCollection;

extern const BoxCharCollection BOX_CHAR_BORDERS[__BORDER_END];

extern byte SCREEN_WIDTH;
extern byte SCREEN_HEIGHT;

extern byte DIALOG_CONTENT_WIDTH;

#define DIALOG_PADDING_X      4
#define DIALOG_PADDING_Y      1
#define DIALOG_WIDTH          SCREEN_WIDTH
#define INDENT_SIZE           DIALOG_PADDING_X

#define TAB                   "   "

#define __callback_prefix     __menu_
/**
 * @brief Gets the choice callback with <name>
 */
#define choice_callback(name) &TOKENPASTE2(__callback_prefix, name)
/**
 * @brief Delare a callback function for a specific choice
 */
#define declare_choice_callback(name) \
  void TOKENPASTE2(__callback_prefix, name)(const struct ChoiceDialogChoice* choice MAYBE_UNUSED)
/**
 * @brief Declare a default callback function
 */
#define declare_choice_callback_g(name) \
  void TOKENPASTE2(__callback_prefix, name)(const struct ChoiceDialogChoice* choice MAYBE_UNUSED, const int index MAYBE_UNUSED)

#pragma once
#include "base.h"
#include "ansi_codes.h"

typedef void (*ChoiceCallback)(const struct ChoiceDialogChoice*);
typedef void (*ChoiceDialogCallback)(const struct ChoiceDialogChoice*, const int);

struct ChoiceDialogChoice
{
	char* name;
	ChoiceCallback callback;
};

enum BorderStyle
{
	BORDER_SINGLE,
	BORDER_DOUBLE
};

typedef struct _BoxOptions
{
	const char* title;
	const int height;
	const char* color;
	const byte paddingX;
	const byte paddingY;
	Coord* captures;
	byte captures_count;
	const bool do_not_free;
} *BoxOptions;

typedef struct _DialogOptions
{
	const char* title;
	const ChoiceDialogCallback callback;
	const char* color;
	const bool noPaddingY;
} *DialogOptions;

typedef byte WrapLineKind;
#define WRAPLINEKIND_LTR 0
#define WRAPLINEKIND_RTL 1
#define WRAPLINEKIND_CENTER 2
#define WRAPLINEKIND_NONE 3

struct WrapLine
{
	byte length;
	byte client_length;
	char text[61];
	WrapLineKind kind;
};

typedef struct _WrapLineOptions
{
	int height;
	Coord* captures;
	byte captures_count;
	struct WrapLine* lines;
	byte* added_count;
	WrapLineKind kind;
} *WrapLineOptions;

// Provides y position of first line and last line
struct _ChoiceInfo
{
	byte start, end;
};

enum ConfirmationDialogReturn
{
	CONFIRMATION_DIALOG_NO,
	CONFIRMATION_DIALOG_YES,
	CONFIRMATION_DIALOG_QUIT = -1
};

// @param options - is optional
struct WrapLine* wrapText(const char* text, int width, const WrapLineOptions options);

/**
 * @brief Convert a string to a single line
 * @returns a new cvector of lines
 */
struct WrapLine* textToLines(const char* text);

/**
 * @brief Convert a string to a single line
 * @param lines - an existing cvector
 * @returns the new pointer to <lines>
 */
struct WrapLine* textToLinesWL(struct WrapLine* lines, const char* text);

/**
 * @brief Wraps <text> and applies a border
 * @param options - is optional
 * @returns a new cvector of lines
 */
struct WrapLine* wrapBox(const char* text, const int width, const BoxOptions options);

/**
 * @brief Wraps <text>, applies a border, and draws it
 * @param border - possible values: BORDER_SINGLE, BORDER_DOUBLE
 * @param options - is optional
 */
void drawBox(const char* text, const int width, const enum BorderStyle border, const BoxOptions options);

/**
 * @brief Applies a border to <lines> and draws it
 * @param border - possible values: BORDER_SINGLE, BORDER_DOUBLE
 * @param options - is optional
 */
void drawBoxWL(struct WrapLine* lines, const int width, const enum BorderStyle border, const BoxOptions options);

/**
 * @brief Wraps <text>
 * @param options - is optional
 */
void showChoiceDialog(const char* text, const struct ChoiceDialogChoice* choices, const int choices_size, const DialogOptions options);

/**
 * @brief Uses cvector of lines
 * @param options - is optional
 */
void showChoiceDialogWL(struct WrapLine* lines, const struct ChoiceDialogChoice* choices, const int choices_size, const DialogOptions options);
// Check for errno afterwards
void showInfoDialog(const char title[], const char text[]);
void showStoryDialog(const char title[], const char text[]);
enum ConfirmationDialogReturn showConfirmationDialog(const char* text);
void showPromptDialog(const char text[], char* buffer, short buffer_size);
void showErrorDialog(const char* context);

/**
 * @brief Draws block of text at <x> and <y>
 */
void putBlock(const char* text, byte x, byte y);

/**
 * @brief Draws lines at <x> and <y>
 */
void putBlockWL(struct WrapLine* lines, byte x, byte y, byte width);

/**
 * @brief Draws block of lines at <x> and <y> and fills background with whitespace
 */
void putBlockWLFill(struct WrapLine* lines, byte x, byte y, byte width);

void indentLines(struct WrapLine* begin, struct WrapLine* end, const byte amount);

struct WrapLine* addNewline(struct WrapLine* lines);
struct WrapLine* justifyLineWL(struct WrapLine* lines, const char* text1, const char* text2, const byte width);
struct WrapLine* addBar(struct WrapLine* lines);

/**
 * @brief Adds a line with <text> to cvector <lines>
 * @returns new pointer to <lines>
 */
struct WrapLine* addLine(struct WrapLine* lines, const char* text, WrapLineKind kind);

/**
 * @brief Adds a line with const char[] text to cvector <lines>
 * @returns nothing
 */
#define addStaticLine(lines, _text, _kind) \
do { \
	cvector_push_back_struct(lines); \
	cvector_last(lines).client_length = (byte)sizeof(_text) - (byte)1; \
	cvector_last(lines).length = (byte)sizeof(_text) - (byte)1; \
	cvector_last(lines).kind = (_kind); \
	memcpy(cvector_last(lines).text, (_text), sizeof(_text)); \
} while (0)

 // box drawing chars
#define BOX_CHAR_D_DR   201
#define BOX_CHAR_D_DL   187
#define BOX_CHAR_D_H    205
#define BOX_CHAR_D_V    186
#define BOX_CHAR_D_UR   200
#define BOX_CHAR_D_UL   188
#define BOX_CHAR_DR     218
#define BOX_CHAR_DL     191
#define BOX_CHAR_H      196
#define BOX_CHAR_V      179
#define BOX_CHAR_UR     192
#define BOX_CHAR_UL     217

extern byte SCREEN_WIDTH;
extern byte SCREEN_HEIGHT;

extern byte DIALOG_CONTENT_WIDTH;

#define DIALOG_PADDING_X 4
#define DIALOG_PADDING_Y 1
#define DIALOG_WIDTH SCREEN_WIDTH
#define INDENT_SIZE DIALOG_PADDING_X

// @brief Signifies a position to capture
#define CONTROL_CHAR (char)5
// @brief Signifies a position to capture
#define CONTROL_CHAR_STR "\5"

#define TAB "   "

#define __callback_prefix __menu_
/**
 * @brief Gets the choice callback with <name>
 */
#define choice_callback(name) &TOKENPASTE2(__callback_prefix, name)
 /**
  * @brief Delare a callback function for a specific choice
  */
#define declare_choice_callback(name) void TOKENPASTE2(__callback_prefix, name)(const struct ChoiceDialogChoice* choice)
  /**
   * @brief Declare a default callback function
   */
#define declare_choice_callback_g(name) void TOKENPASTE2(__callback_prefix, name)(const struct ChoiceDialogChoice* choice, const int index)
#pragma once
#include "base.h"

typedef void (*ChoiceCallback)(const struct ChoiceDialogChoice*);
typedef void (*ChoiceDialogCallback)(const struct ChoiceDialogChoice*, const int);

struct ChoiceDialogChoice
{
	const char* name, * description;
	const ChoiceCallback callback;
};

enum BorderStyle
{
	BORDER_SINGLE,
	BORDER_DOUBLE
};

typedef struct
{
	byte x, y;
} Coord;

typedef struct _BoxOptions
{
	const char* title;
	const int height;
	const char* color;
	const int paddingX;
	const int paddingY;
	Coord* captures;
} *BoxOptions;

typedef struct _DialogOptions
{
	const char* title;
	const ChoiceDialogCallback callback;
	const char* color;
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
	struct WrapLine* lines;
	byte* added_count;
} *WrapLineOptions;

struct _ChoiceInfo
{
	byte start, end;
};

struct WrapLine* wrapText(const char* text, int width, const WrapLineOptions options);
struct WrapLine* textToLines(const char* text);
struct WrapLine* textToLinesWL(struct WrapLine* lines, const char* text);
void drawBoxWL(struct WrapLine* lines, const int width, const enum BorderStyle border, const BoxOptions options);
void drawBox(const char* text, const int width, const enum BorderStyle border, const BoxOptions options);
void showChoiceDialog(const char* text, const char* prompt, const struct ChoiceDialogChoice* choices, const int choices_size, const DialogOptions options);
void showInfoDialog(const char title[], const char text[]);
void putBlock(const char* text, byte x, byte y);
void putBlockWL(const struct WrapLine* lines, byte x, byte y);
struct WrapLine* addNewline(struct WrapLine* lines);
struct WrapLine* addLine(struct WrapLine* lines, const char* text, WrapLineKind kind);
struct WrapLine* justifyLineWL(struct WrapLine* lines, const char* text1, const char* text2, const byte width);

#define addStaticLine(lines, _text, _kind) \
	cvector_push_back_struct(lines); \
	cvector_last(lines).client_length = (byte)sizeof(_text) - (byte)1; \
	cvector_last(lines).length = (byte)sizeof(_text) - (byte)1; \
	cvector_last(lines).kind = _kind; \
	memcpy(cvector_last(lines).text, _text, sizeof(_text));

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

#define ANSI_COLOR_RED         "\033[31m"
#define ANSI_COLOR_GREEN       "\033[32m"
#define ANSI_COLOR_YELLOW      "\033[33m"
#define ANSI_COLOR_BLUE        "\033[34m"
#define ANSI_COLOR_MAGENTA     "\033[35m"
#define ANSI_COLOR_CYAN        "\033[36m"
#define ANSI_COLOR_BROWN       "\033[38;5;94m"
#define ANSI_SELECTED	       "\033[30;47m"
#define ANSI_COLOR_RESET       "\033[0m"
#define ANSI_BG_RESET          "\033[40m"
#define ANSI_CURSOR_SHOW	   "\033[?25h"
#define ANSI_CURSOR_HIDE	   "\033[?25l"
#define ANSI_CURSOR_SAVE       "\033[s"
#define ANSI_CURSOR_RESTORE    "\033[u"
#define ANSI_CURSOR_POS(x, y)  "\033[" x ";" y "H"
#define ANSI_CURSOR_STYLE_UNDERLINE "\033[3q"
#define ANSI_CURSOR_STYLE_DEFAULT "\033[0q"
#define ANSI_BG_BROWN    "\033[48;5;94m"
#define ANSI_BOLD "\033[4m"

#define DIALOG_PADDING_X 4
#define DIALOG_PADDING_Y 1
#define DIALOG_CONTENT_WIDTH 32
#define DIALOG_WIDTH DIALOG_CONTENT_WIDTH + DIALOG_PADDING_X * 2

#define CONTROL_CHAR (char)5
#define CONTROL_CHAR_STR "\5"

#define choice_callback(name) static void menu_ ## name(const struct ChoiceDialogChoice* choice)
// Default callback for all choices
#define choice_callback_g(name) static void menu_ ## name(const struct ChoiceDialogChoice* choice, const int index)
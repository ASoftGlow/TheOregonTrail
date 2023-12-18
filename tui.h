#pragma once
#include "base.h"

typedef void (*ChoiceCallback)(const struct ChoiceDialogChoice*);
typedef void (*ChoiceDialogCallback)(const struct ChoiceDialogChoice*, const int);

struct ChoiceDialogChoice
{
	const char* name, * description;
	const ChoiceCallback callback;
};

enum BorderStyle {
	BORDER_SINGLE,
	BORDER_DOUBLE
};

typedef struct {
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

struct WrapLine {
	byte length;
	byte client_length;
	char text[62];
};

typedef struct _WrapLineOptions
{
	int height;
	Coord* captures;
	struct WrapLine* plines;
	byte* lines_count;
} *WrapLineOptions;

struct _ChoiceInfo
{
	byte start, end;
};

struct WrapLine* wrapText(const char* text, int width, const WrapLineOptions options);
void drawBox_wl(struct WrapLine* lines, const int width, const enum BorderStyle border, const BoxOptions options);
void drawBox(const char* text, const int width, const enum BorderStyle border, const BoxOptions options);
void showChoiceDialog(const char* text, const char* prompt, const struct ChoiceDialogChoice* choices, const int choices_size, const DialogOptions options);
void showInfoDialog(const char title[], const char text[]);

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
#define ANSI_SELECTED	       "\033[30;47m"
#define ANSI_COLOR_RESET       "\033[0m"
#define ANSI_CURSOR_SHOW	   "\033[?25h"
#define ANSI_CURSOR_HIDE	   "\033[?25l"
#define ANSI_CURSOR_SAVE       "\033[s"
#define ANSI_CURSOR_RESTORE    "\033[u"
#define ANSI_CURSOR_POS(x, y)  "\033[" x ";" y "H"

#define DIALOG_PADDING_X 4
#define DIALOG_PADDING_Y 1
#define DIALOG_WIDTH 32 + DIALOG_PADDING_X * 2

#define choice_callback(name) void menu_ ## name(const struct ChoiceDialogChoice* choice)
// Default callback for all choices
#define choice_callback_g(name) void menu_ ## name(const struct ChoiceDialogChoice* choice, const int index)
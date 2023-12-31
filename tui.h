#pragma once
#include "base.h"
#include "ansi_codes.h"

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
	bool do_not_free;
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
	WrapLineKind kind;
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
void putBlockWL(struct WrapLine* lines, byte x, byte y, byte width);
void putBlockWLFill(struct WrapLine* lines, byte x, byte y, byte width);
struct WrapLine* addNewline(struct WrapLine* lines);
struct WrapLine* addLine(struct WrapLine* lines, const char* text, WrapLineKind kind);
struct WrapLine* justifyLineWL(struct WrapLine* lines, const char* text1, const char* text2, const byte width);
struct WrapLine* addBar(struct WrapLine* lines);

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

#define DIALOG_PADDING_X 4
#define DIALOG_PADDING_Y 1
#define DIALOG_CONTENT_WIDTH 32
#define DIALOG_WIDTH DIALOG_CONTENT_WIDTH + DIALOG_PADDING_X * 2

#define CONTROL_CHAR (char)5
#define CONTROL_CHAR_STR "\5"

#define __callback_prefix __menu_
#define choice_callback(name) &TOKENPASTE2(__callback_prefix, name)
#define declare_choice_callback(name) void TOKENPASTE2(__callback_prefix, name)(const struct ChoiceDialogChoice* choice)
// Default callback for all choices
#define declare_choice_callback_g(name) void TOKENPASTE2(__callback_prefix, name)(const struct ChoiceDialogChoice* choice, const int index)
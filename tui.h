#pragma once

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

void drawBox(const int width, int height, const char* text, const char title[], const enum BorderStyle border, const char color[], const int paddingX, const int paddingY);
void showChoiceDialog(const char title[], const char* text, const char* prompt, const struct ChoiceDialogChoice* choices, const int choices_size, const ChoiceDialogCallback callback, const char color[]);
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

#define DIALOG_PADDING_X 4
#define DIALOG_PADDING_Y 1
#define DIALOG_WIDTH 32 + DIALOG_PADDING_X * 2

#define choice_callback(name) void menu_ ## name(const struct ChoiceDialogChoice* choice)
// Default callback for all choices
#define choice_callback_g(name) void menu_ ## name(const struct ChoiceDialogChoice* choice, const int index)
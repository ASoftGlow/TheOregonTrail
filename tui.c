#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "tui.h"
#include "constants.h"
#include "utils.h"
#include "input.c"
#include <stdarg.h>

void drawChoice(const struct ChoiceDialogChoice* choice, const int index, const int selected);

char down[] = "\033[?B";
char forward[] = "\033[?C";

static enum QKeyCallbackReturn inputCallback(int key, enum QKeyType type, va_list args)
{
	int* cur_pos = va_arg(args, int*);
	const int choices_size = va_arg(args, const int);
	const struct ChoiceDialogChoice* choices = va_arg(args, const struct ChoiceDialogChoice*);
	const ChoiceDialogCallback callback = va_arg(args, const ChoiceDialogCallback);
	va_end(args);

	if (type == QKEY_TYPE_ARROW)
	{
		if (key == QARROW_DOWN && *cur_pos + 1 != choices_size)
		{
			if (*cur_pos != -1)
				drawChoice(&choices[*cur_pos], *cur_pos, 0);
			else
				puts_n(ANSI_CURSOR_HIDE);

			drawChoice(&choices[++*cur_pos], *cur_pos, 1);
		}
		else if (key == QARROW_UP && *cur_pos != 0)
		{
			if (*cur_pos != -1)
				drawChoice(&choices[*cur_pos], *cur_pos, 0);
			else
			{
				*cur_pos = choices_size;
				puts_n(ANSI_CURSOR_HIDE);
			}
			drawChoice(&choices[--*cur_pos], *cur_pos, 1);
		}
	}
	else
	{
		if (key == '\r' && *cur_pos != -1)
		{
			const struct ChoiceDialogChoice* choice = &choices[*cur_pos];
			if (choice->callback) (*choice->callback)(choice);
			else if (callback) (*callback)(choice, *cur_pos);
		}
		else if (key == 27 && *cur_pos > -1)
		{
			drawChoice(&choices[*cur_pos], *cur_pos, 0);
			*cur_pos = -1;
			puts_n(ANSI_CURSOR_SHOW);
		}
	}

	return *cur_pos == -1 ? QKEY_CALLBACK_RETURN_NORMAL : QKEY_CALLBACK_RETURN_IGNORE;
}

void showChoiceDialog(const char title[], const char* text, const char* prompt, const struct ChoiceDialogChoice* choices, const int choices_size, const ChoiceDialogCallback callback, const char color[])
{
	//const int str_size = strlen(title) + 2 + 1;
	//char* str = malloc(str_size);
	char str[256] = { 0 }; // TODO: dynamic
	strcat(str, text);
	strcat(str, "\n" ANSI_CURSOR_SAVE);
	for (int i = 0; i < choices_size; i++)
	{
		const struct ChoiceDialogChoice choice = choices[i];
		strcat(str, "\n");
		strcat_ch(str, '1' + i);
		strcat(str, ". ");
		strcat(str, choice.name);
		if (choice.description)
		{
			strcat(str, "\t");
			strcat(str, choice.description);
		}
	}
	strcat(str, "\n\n");
	strcat(str, prompt);

	clear_stdout();
	drawBox(DIALOG_WIDTH, 0, str, title, BORDER_DOUBLE, color ? color : ANSI_COLOR_YELLOW, DIALOG_PADDING_X, DIALOG_PADDING_Y);

	//free(str);
	int cur_pos = -1;
	const int num = vgetNumber(1, choices_size, &inputCallback, &cur_pos, choices_size, choices, callback) - 1;
	const struct ChoiceDialogChoice* choice = &choices[num];
	if (choice->callback) (*choice->callback)(choice);
	else if (callback) (*callback)(choice, num);

}

void drawChoice(const struct ChoiceDialogChoice* choice, const int index, const int selected)
{
	down[2] = '1' + index;
	puts_n(ANSI_CURSOR_RESTORE);
	puts_n(down);
	if (selected) puts_n(ANSI_SELECTED);
	putchar('1' + index);
	puts_n(". ");
	puts_n(choice->name);
	if (choice->description)
	{
		putchar('\t');
		puts_n(choice->description);
	}
	if (selected) puts_n(ANSI_COLOR_RESET);
}


void drawBox(const int width, const int height, const char* old_text, const char title[], const enum BorderStyle border, const char color[], const int paddingX, const int paddingY)
{
	const size_t old_text_size = strlen(old_text) + 1;
	char* text = malloc(old_text_size);
	assert(text);
	memcpy(text, old_text, old_text_size);

	if (title) assert(*title && strlen(title) + 2 <= width);
	int w = width, h = height ? height - paddingY * 2 : 1;

	const char BORDER_V = border ? BOX_CHAR_D_V : BOX_CHAR_V;
	const char BORDER_H = border ? BOX_CHAR_D_H : BOX_CHAR_H;
	const char BORDER_DR = border ? BOX_CHAR_D_DR : BOX_CHAR_DR;
	const char BORDER_DL = border ? BOX_CHAR_D_DL : BOX_CHAR_DL;
	const char BORDER_UR = border ? BOX_CHAR_D_UR : BOX_CHAR_UR;
	const char BORDER_UL = border ? BOX_CHAR_D_UL : BOX_CHAR_UL;

	if (color) puts_n(color);
	putchar(BORDER_DR);
	if (title)
	{
		const int l = width - (int)strlen(title) - 2;
		const int seg = w = (int)(l / 2);
		while (w--) putchar(BORDER_H);
		putchar(' ');
		puts_n(title);
		putchar(' ');
		w = seg;
		if (l % 2) w++;
		while (w--) putchar(BORDER_H);
	}
	else
		while (w--) putchar(BORDER_H);
	putchar(BORDER_DL);
	putchar('\n');

	int py = paddingY;
	while (py--)
	{
		putchar(BORDER_V);
		if (color) puts_n(ANSI_COLOR_RESET);
		w = width;
		while (w--) putchar(' ');
		if (color) puts_n(color);
		putchar(BORDER_V);
		putchar('\n');
	}

	int progress = 0;
	char* ch = strtok(text, " \n");
	while (h--)
	{
		if (height == 0 && ch == 0) break;
		putchar(BORDER_V);
		if (color) puts_n(ANSI_COLOR_RESET);
		w = width - paddingX;
		int p = paddingX;
		while (p--) putchar(' ');

		while (ch && w)
		{
			int length = (int)strlen_iae(ch);
			int real_length = (int)strlen(ch);
			if (w != width - paddingX)
			{
				length++;
				real_length++;
			}
			if (progress && old_text[progress] == '\n' && *ch)
			{
				progress++;
				if (height == 0) h++;
				break;
			}
			if (h == 0)
			{
				if (height == 0)
				{
					h++;
					goto dynamic_height_continue;
				}
				if (length > w - 4 - paddingX) {
					puts_n("...");
					ch = 0;
					w -= 3;
					break;
				}
			}
			else
			{
			dynamic_height_continue:
				if (length > w - paddingX)
				{
					progress++;
					break;
				}
			}
			if (w != width - paddingX) putchar(' ');
			progress += real_length;
			puts_n(ch);
			w -= length;
			if (w == 0) progress++;
			ch = strtok(0, " \n");
		}

		while (w--) putchar(' ');
		if (color) puts_n(color);
		putchar(BORDER_V);
		putchar('\n');
	}

	py = paddingY;
	while (py--)
	{
		putchar(BORDER_V);
		if (color) puts_n(ANSI_COLOR_RESET);
		w = width;
		while (w--) putchar(' ');
		if (color) puts_n(color);
		putchar(BORDER_V);
		putchar('\n');
	}

	putchar(BORDER_UR);
	w = width;
	while (w--) putchar(BORDER_H);
	putchar(BORDER_UL);
	if (color) puts_n(ANSI_COLOR_RESET);
	putchar('\n');

	fflush(stdout);
	free(text);
}

const char press_space[] = "\n\n" ANSI_COLOR_CYAN "Press SPACE BAR to continue" ANSI_COLOR_RESET;

void showInfoDialog(const char title[], const char text[])
{
	const size_t text_len = strlen(text);
	char* _text = malloc(text_len + sizeof(press_space));
	assert(_text);
	memcpy(_text, text, text_len);
	memcpy(_text + text_len, press_space, sizeof(press_space));

	puts_n(ANSI_CURSOR_HIDE);
	clear_stdout();
	drawBox(DIALOG_WIDTH, 0, _text, title, BORDER_SINGLE, ANSI_COLOR_YELLOW, DIALOG_PADDING_X, DIALOG_PADDING_Y);

	fflush(stdout);
	free(_text);

	waitForKey(' ');
	puts_n(ANSI_CURSOR_SHOW);
}
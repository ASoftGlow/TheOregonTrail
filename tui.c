#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "libs/cvector.h"

#include "tui.h"
#include "utils.h"
#include "input.c"

void drawChoice(const struct ChoiceDialogChoice* choices, const struct WrapLine* lines, const struct _ChoiceInfo* choices_info, const int index, const bool selected);

struct WrapLine* wrapText(const char* text, int width, const WrapLineOptions options)
{
	struct WrapLine* lines = 0;
	if (options && options->plines)
	{
		lines = options->plines;
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

	const bool auto_height = options && options->height;

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
			lines[l].text[0] = 0;
			escaped_chars = 0;
			continue;
		}
		else if (text[i] == 27) is_escaping = 1;
		else if (text[i] == ' ') last_break = i;
		else if (text[i] == 5 && options)
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
			lines[l].text[lines[l].length -= i - last_break] = 0;
			i = last_break;
			goto newline;
		}
	}
	lines[l].client_length = lines[l].length - escaped_chars;
	lines[l].text[lines[l].length++] = 0;

	if (options && options->lines_count) *options->lines_count = l - start + (byte)1;
	return lines;
}

static struct WrapLine* addNewline(struct WrapLine* lines)
{
	cvector_push_back_struct(lines);
	const byte i = (byte)cvector_size(lines) - (byte)1;
	lines[i].length = 0;
	lines[i].client_length = 0;
	lines[i].text[0] = 0;
	return lines;
}

static enum QKeyCallbackReturn inputCallback(int key, enum QKeyType type, va_list args)
{
	int* cur_pos = va_arg(args, int*);
	const int choices_size = va_arg(args, const int);
	const struct ChoiceDialogChoice* choices = va_arg(args, const struct ChoiceDialogChoice*);
	const ChoiceDialogCallback callback = va_arg(args, const ChoiceDialogCallback);
	const struct WrapLine* lines = va_arg(args, const struct WrapLine*);
	const struct _ChoiceInfo* choices_info = va_arg(args, const struct _ChoiceInfo*);
	const Coord end = va_arg(args, const Coord);
	va_end(args);

	if (type == QKEY_TYPE_ARROW)
	{
		if (key == QARROW_DOWN && *cur_pos + 1 != choices_size)
		{
			if (*cur_pos != -1)
				drawChoice(choices, lines, choices_info, *cur_pos, 0);
			else
				puts_n(ANSI_CURSOR_HIDE);

			drawChoice(choices, lines, choices_info, ++ * cur_pos, 1);
		}
		else if (key == QARROW_UP && *cur_pos != 0)
		{
			if (*cur_pos != -1)
				drawChoice(choices, lines, choices_info, *cur_pos, 0);
			else
			{
				*cur_pos = choices_size;
				puts_n(ANSI_CURSOR_HIDE);
			}
			drawChoice(choices, lines, choices_info, -- * cur_pos, 1);
		}
	}
	else
	{
		if (key == '\r' && *cur_pos != -1)
		{
			puts_n(ANSI_CURSOR_SHOW);
			const struct ChoiceDialogChoice* choice = &choices[*cur_pos];
			if (choice->callback) (*choice->callback)(choice);
			else if (callback) (*callback)(choice, *cur_pos);
		}
		else if (key == 27 && *cur_pos > -1)
		{
			drawChoice(choices, lines, choices_info, *cur_pos, 0);
			*cur_pos = -1;
			set_cursor_pos(end.x, end.y);
			puts_n(ANSI_CURSOR_SHOW);
		}
	}

	return *cur_pos == -1 ? QKEY_CALLBACK_RETURN_NORMAL : QKEY_CALLBACK_RETURN_IGNORE;
}

void showChoiceDialog(const char* text, const char* prompt, const struct ChoiceDialogChoice* choices, const int choices_size, const DialogOptions options)
{
	Coord captures[1];
	struct _ChoiceInfo* choices_info = (struct _ChoiceInfo*)malloc(sizeof(struct _ChoiceInfo) * choices_size);
	assert(choices_info);
	struct WrapLine* lines = wrapText(text, DIALOG_WIDTH - DIALOG_PADDING_X * 2, 0);
	lines = addNewline(lines);

	for (int i = 0; i < choices_size; i++)
	{
		byte added_count = 0;
#pragma warning( once : 6011 )
		char str[64] = { '1' + i, ". " };
		strcat(str, choices[i].name);
		if (choices[i].description)
		{
			strcat(str, " - ");
			strcat(str, choices[i].description);
		}
		choices_info[i].start = (byte)cvector_size(lines);
		lines = wrapText(str, DIALOG_WIDTH - DIALOG_PADDING_X * 2, &(struct _WrapLineOptions){
			.plines = lines,
				.lines_count = &added_count
		});
		choices_info[i].end = (byte)choices_info[i].start + added_count;
	}
	lines = addNewline(lines);

	const size_t len = strlen(prompt);
	char* str2 = malloc(sizeof(char) * (len + sizeof(" \005")));
	assert(str2);
	memcpy(str2, prompt, len);
	strcpy(str2 + len, " \005");
	lines = wrapText(str2, DIALOG_WIDTH - DIALOG_PADDING_X * 2, &(struct _WrapLineOptions){
		.plines = lines,
			.captures = &captures
	});
	free(str2);
	captures[0].x += 1 + DIALOG_PADDING_X;
	captures[0].y += 2;

	clear_stdout();
	drawBox_wl(lines, DIALOG_WIDTH, BORDER_DOUBLE, &(struct _BoxOptions) {
		.title = options->title,
			.color = options->color ? options->color : ANSI_COLOR_YELLOW,
			.paddingX = DIALOG_PADDING_X,
			.paddingY = DIALOG_PADDING_Y
	});
	puts_n(ANSI_CURSOR_SAVE);

	set_cursor_pos(captures[0].x, captures[0].y);
	int cur_pos = -1;
	const int num = vgetNumber(1, choices_size, &inputCallback, &cur_pos, choices_size, choices, options->callback, lines, choices_info, captures[0]) - 1;

	const struct ChoiceDialogChoice* choice = &choices[num];
	if (choice->callback) (*choice->callback)(choice);
	else if (options->callback) (*options->callback)(choice, num);

	free(choices_info);
	cvector_free(lines);
}

void drawChoice(const struct ChoiceDialogChoice* choices, const struct WrapLine* lines, const struct _ChoiceInfo* choices_info, const int index, const bool selected)
{
	byte x = 1 + DIALOG_PADDING_X;
	byte y = 1 + DIALOG_PADDING_Y;
	puts_n(selected ? ANSI_SELECTED : ANSI_COLOR_RESET);
	for (byte i = choices_info[index].start; i < choices_info[index].end; i++)
	{
		set_cursor_pos(x, y + i);
		puts_n(lines[i].text);
	}
	if (selected) puts_n(ANSI_COLOR_RESET);
}

void drawBox(const char* text, const int width, const enum BorderStyle border, const BoxOptions options)
{
	struct WrapLine* lines = wrapText(text, width - options->paddingX * 2, &(struct _WrapLineOptions){
		.height = options->height - options->paddingY * 2,
			.captures = options->captures
	});

	drawBox_wl(lines, width, border, options);
}

void drawBox_wl(struct WrapLine* lines, const int width, const enum BorderStyle border, const BoxOptions options)
{
	if (options->title) assert(*options->title && strlen(options->title) + 2 <= width);
	int w = width;

	const char BORDER_V = border ? BOX_CHAR_D_V : BOX_CHAR_V;
	const char BORDER_H = border ? BOX_CHAR_D_H : BOX_CHAR_H;
	const char BORDER_DR = border ? BOX_CHAR_D_DR : BOX_CHAR_DR;
	const char BORDER_DL = border ? BOX_CHAR_D_DL : BOX_CHAR_DL;
	const char BORDER_UR = border ? BOX_CHAR_D_UR : BOX_CHAR_UR;
	const char BORDER_UL = border ? BOX_CHAR_D_UL : BOX_CHAR_UL;

	if (options->color) puts_n(options->color);
	putchar(BORDER_DR);
	if (options->title)
	{
		const int l = width - (int)strlen(options->title) - 2;
		const int seg = w = (int)(l / 2);
		while (w--) putchar(BORDER_H);
		putchar(' ');
		puts_n(options->title);
		putchar(' ');
		w = seg;
		if (l % 2) w++;
		while (w--) putchar(BORDER_H);
	}
	else
		while (w--) putchar(BORDER_H);
	putchar(BORDER_DL);
	putchar('\n');

	int py = options->paddingY;
	while (py--)
	{
		putchar(BORDER_V);
		if (options->color) puts_n(ANSI_COLOR_RESET);
		w = width;
		while (w--) putchar(' ');
		if (options->color) puts_n(options->color);
		putchar(BORDER_V);
		putchar('\n');
	}

	for (byte l = 0; l < cvector_size(lines); l++)
	{
		putchar(BORDER_V);
		if (options->color) puts_n(ANSI_COLOR_RESET);
		int p = options->paddingX;
		while (p--) putchar(' ');
		puts_n(lines[l].text);
		p = width - options->paddingX - lines[l].client_length;
		while (p--) putchar(' ');
		if (options->color) puts_n(options->color);
		putchar(BORDER_V);
		putchar('\n');
	}

	py = options->paddingY;
	while (py--)
	{
		putchar(BORDER_V);
		if (options->color) puts_n(ANSI_COLOR_RESET);
		w = width;
		while (w--) putchar(' ');
		if (options->color) puts_n(options->color);
		putchar(BORDER_V);
		putchar('\n');
	}

	putchar(BORDER_UR);
	w = width;
	while (w--) putchar(BORDER_H);
	putchar(BORDER_UL);
	if (options->color) puts_n(ANSI_COLOR_RESET);
	putchar('\n');

	fflush(stdout);
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
	drawBox(_text, DIALOG_WIDTH, BORDER_SINGLE, &(struct _BoxOptions){
		.title = title,
			.color = ANSI_COLOR_YELLOW,
			.paddingX = DIALOG_PADDING_X,
			.paddingY = DIALOG_PADDING_Y
	});

	fflush(stdout);
	free(_text);

	waitForKey(' ');
	puts_n(ANSI_CURSOR_SHOW);
}
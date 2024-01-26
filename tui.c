#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "libs/cvector.h"

#include "tui.h"
#include "utils.h"
#include "input.h"

void drawChoice(const struct ChoiceDialogChoice* choices, const struct WrapLine* lines, const struct _ChoiceInfo* choices_info, const int index, const bool selected);

struct WrapLine* wrapText(const char* text, int width, const WrapLineOptions options)
{
	struct WrapLine* lines = NULL;
	if (options && options->lines)
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
	lines[l].kind = options ? options->kind : WRAPLINEKIND_LTR;

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
			lines[l].kind = options ? options->kind : WRAPLINEKIND_LTR;
			lines[l].text[0] = 0;
			escaped_chars = 0;
			continue;
		}
		else if (text[i] == ESCAPE_CHAR) is_escaping = 1;
		else if (text[i] == ' ') last_break = i;
		else if (text[i] == CONTROL_CHAR && options)
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

	if (options && options->added_count) *options->added_count = l - start + (byte)1;
	return lines;
}

struct WrapLine* addNewline(struct WrapLine* lines)
{
	cvector_push_back_struct(lines);
	struct WrapLine* last_line = &cvector_last(lines);
	last_line->length = 0;
	last_line->client_length = 0;
	last_line->text[0] = 0;
	last_line->kind = WRAPLINEKIND_LTR;
	return lines;
}

struct WrapLine* addLine(struct WrapLine* lines, const char* text, WrapLineKind kind)
{
	cvector_push_back_struct(lines);
	struct WrapLine* last_line = &cvector_last(lines);
	last_line->length = (byte)strlen(text);
	last_line->client_length = (byte)_strlen_iae(text);
	last_line->kind = kind;
	if (text)
		memcpy(last_line->text, text, (size_t)last_line->length + 1);
	return lines;
}

struct WrapLine* justifyLineWL(struct WrapLine* lines, const char* text1, const char* text2, const byte width)
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

static enum QKeyCallbackReturn inputCallback(unsigned key, enum QKeyType type, va_list args)
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
		switch (key)
		{
		case QARROW_DOWN:
			if (*cur_pos == choices_size - 1) break;
			if (*cur_pos != -1)
				drawChoice(choices, lines, choices_info, *cur_pos, 0);
			else
				putsn(ANSI_CURSOR_HIDE);

			drawChoice(choices, lines, choices_info, ++ * cur_pos, 1);
			break;

		case QARROW_UP:
			if (*cur_pos == 0) break;
			if (*cur_pos != -1)
				drawChoice(choices, lines, choices_info, *cur_pos, 0);
			else
			{
				*cur_pos = choices_size;
				putsn(ANSI_CURSOR_HIDE);
			}
			drawChoice(choices, lines, choices_info, -- * cur_pos, 1);
			break;

		case QARROW_PAGE_DOWN:
			if (*cur_pos == -1 || *cur_pos == choices_size - 1) break;
			drawChoice(choices, lines, choices_info, *cur_pos, 0);
			*cur_pos = choices_size - 1;
			drawChoice(choices, lines, choices_info, *cur_pos, 1);
			break;

		case QARROW_PAGE_UP:
			if (*cur_pos <= 0) break;
			drawChoice(choices, lines, choices_info, *cur_pos, 0);
			*cur_pos = 0;
			drawChoice(choices, lines, choices_info, *cur_pos, 1);
			break;
		}
	}
	else
	{
		if (key == '\r' && *cur_pos != -1)
		{
			putsn(ANSI_CURSOR_SHOW);
			const struct ChoiceDialogChoice* choice = &choices[*cur_pos];
			if (choice->callback) (*choice->callback)(choice);
			else if (callback) (*callback)(choice, *cur_pos);
			return QKEY_CALLBACK_RETURN_END;
		}
		else if (key == ESCAPE_CHAR && *cur_pos > -1)
		{
			drawChoice(choices, lines, choices_info, *cur_pos, 0);
			*cur_pos = -1;
			setCursorPos(end.x, end.y);
			putsn(ANSI_CURSOR_SHOW);
			fflush(stdout);
		}
	}

	return *cur_pos == -1 ? QKEY_CALLBACK_RETURN_NORMAL : QKEY_CALLBACK_RETURN_IGNORE;
}

void showChoiceDialog(const char* text, const struct ChoiceDialogChoice* choices, const int choices_size, const DialogOptions options)
{
	struct WrapLine* lines = wrapText(text, DIALOG_CONTENT_WIDTH, NULL);
	showChoiceDialogWL(lines, choices, choices_size, options);
}

void showChoiceDialogWL(struct WrapLine* lines, const struct ChoiceDialogChoice* choices, const int choices_size, const DialogOptions options)
{
	const int paddingY = options && options->noPaddingY ? 0 : DIALOG_PADDING_Y;
	Coord capture;
	struct _ChoiceInfo* choices_info = (struct _ChoiceInfo*)malloc(sizeof(struct _ChoiceInfo) * choices_size);
	assert(choices_info);
	lines = addNewline(lines);

	for (int i = 0; i < choices_size; i++)
	{
		byte added_count = 0;
		char str[64] = { '1' + i, '.', ' ', 0 };
		strcat(str, choices[i].name);
		choices_info[i].start = (byte)cvector_size(lines);
		lines = wrapText(str, DIALOG_CONTENT_WIDTH, &(struct _WrapLineOptions){
			.lines = lines,
				.added_count = &added_count
		});
		choices_info[i].end = (byte)choices_info[i].start + added_count;
	}
	lines = addNewline(lines);

	char str2[] = "What is your choice? "CONTROL_CHAR_STR;
	lines = wrapText(str2, DIALOG_CONTENT_WIDTH, &(struct _WrapLineOptions){
		.lines = lines,
			.captures = &capture
	});
	capture.x += 1 + DIALOG_PADDING_X;
	capture.y += 1 + paddingY;

	clearStdout();
	drawBoxWL(lines, DIALOG_WIDTH, BORDER_DOUBLE, &(struct _BoxOptions) {
		.title = options ? options->title : NULL,
			.color = options && options->color ? options->color : ANSI_COLOR_YELLOW,
			.paddingX = DIALOG_PADDING_X,
			.paddingY = paddingY,
			.do_not_free = 1
	});
	putsn(ANSI_CURSOR_SAVE);
	setCursorPos(capture.x, capture.y);
	fflush(stdout);

	int cur_pos = -1;
	const int num = getNumberInput(1, choices_size, 0, &inputCallback, &cur_pos, choices_size, choices, options ? options->callback : NULL, lines, choices_info, capture) - 1;

	cvector_free(lines);
	free(choices_info);

	if (num >= 0)
	{
		const struct ChoiceDialogChoice* choice = &choices[num];
		if (choice->callback) (*choice->callback)(choice);
		else if (options && options->callback) (*options->callback)(choice, num);
	}
}

void drawChoice(const struct ChoiceDialogChoice* choices, const struct WrapLine* lines, const struct _ChoiceInfo* choices_info, const int index, const bool selected)
{
	const byte x = 1 + DIALOG_PADDING_X;
	const byte y = 1 + DIALOG_PADDING_Y; // FIXME: don't assume padding
	if (selected) putsn(ANSI_SELECTED);
	for (byte i = choices_info[index].start; i < choices_info[index].end; i++)
	{
		setCursorPos(x, y + i);
		putsn(lines[i].text);
	}
	if (selected) putsn(ANSI_COLOR_RESET);
	fflush(stdout);
}

struct WrapLine* wrapBox(const char* text, const int width, const BoxOptions options)
{
	return wrapText(text, width - options->paddingX * 2, &(struct _WrapLineOptions){
		.height = options->height - options->paddingY * 2,
			.captures = options->captures
	});
}

void drawBox(const char* text, const int width, const enum BorderStyle border, const BoxOptions options)
{
	drawBoxWL(wrapBox(text, width, options), width, border, options);
}

void drawBoxWL(struct WrapLine* lines, const int width, const enum BorderStyle border, const BoxOptions options)
{
	if (options && options->title) assert(*options->title && strlen(options->title) + 2 <= width);
	int w = width;

	const char BORDER_V = border ? BOX_CHAR_D_V : BOX_CHAR_V;
	const char BORDER_H = border ? BOX_CHAR_D_H : BOX_CHAR_H;
	const char BORDER_DR = border ? BOX_CHAR_D_DR : BOX_CHAR_DR;
	const char BORDER_DL = border ? BOX_CHAR_D_DL : BOX_CHAR_DL;
	const char BORDER_UR = border ? BOX_CHAR_D_UR : BOX_CHAR_UR;
	const char BORDER_UL = border ? BOX_CHAR_D_UL : BOX_CHAR_UL;

	if (options && options->color) putsn(options->color);
	putchar(BORDER_DR);
	if (options && options->title)
	{
		const int l = width - (int)strlen(options->title) - 2;
		const int seg = w = (int)(l / 2);
		while (w--) putchar(BORDER_H);
		putchar(' ');
		if (options->color) putsn(ANSI_COLOR_RESET);
		putsn(options->title);
		if (options->color) putsn(options->color);
		putchar(' ');
		w = seg;
		if (l % 2) w++;
		while (w--) putchar(BORDER_H);
	}
	else
		while (w--) putchar(BORDER_H);
	putchar(BORDER_DL);
	putchar('\n');

	int py = options ? options->paddingY : 0;
	while (py--)
	{
		putchar(BORDER_V);
		if (options && options->color) putsn(ANSI_COLOR_RESET);
		w = width;
		while (w--) putchar(' ');
		if (options && options->color) putsn(options->color);
		putchar(BORDER_V);
		putchar('\n');
	}

	for (byte l = 0; l < cvector_size(lines); l++)
	{
		putchar(BORDER_V);
		if (options && options->color) putsn(ANSI_COLOR_RESET);
		int p;
		switch (lines[l].kind)
		{
		case WRAPLINEKIND_NONE:
			p = options ? options->paddingX : 0;
			while (p--) putchar(' ');
			putsn(lines[l].text);
			p = options ? options->paddingX : 0;
			while (p--) putchar(' ');
			break;

		case WRAPLINEKIND_CENTER:
			const int space = width - lines[l].client_length;
			p = space / 2;
			while (p--) putchar(' ');
			putsn(lines[l].text);
			p = space / 2 + (space & 1);
			while (p--) putchar(' ');
			break;

		case WRAPLINEKIND_RTL:
			p = width - (options ? options->paddingX : 0) - lines[l].client_length;
			while (p--) putchar(' ');
			putsn(lines[l].text);
			p = options ? options->paddingX : 0;
			while (p--) putchar(' ');
			break;

		case WRAPLINEKIND_LTR:
		default:
			p = options ? options->paddingX : 0;
			while (p--) putchar(' ');
			putsn(lines[l].text);
			p = width - (options ? options->paddingX : 0) - lines[l].client_length;
			while (p--) putchar(' ');
			break;
		}

		if (options && options->color) putsn(options->color);
		putchar(BORDER_V);
		putchar('\n');
		fflush(stdout);
	}

	py = options ? options->paddingY : 0;
	while (py--)
	{
		putchar(BORDER_V);
		if (options && options->color) putsn(ANSI_COLOR_RESET);
		w = width;
		while (w--) putchar(' ');
		if (options && options->color) putsn(options->color);
		putchar(BORDER_V);
		putchar('\n');
	}

	putchar(BORDER_UR);
	w = width;
	while (w--) putchar(BORDER_H);
	putchar(BORDER_UL);
	if (options && options->color) putsn(ANSI_COLOR_RESET);
	putchar('\n');

	fflush(stdout);
	if (!(options && options->do_not_free)) cvector_free(lines);
	//if (options && options->captures) TODO: offest all captures
}

const char press_space[] = ANSI_COLOR_CYAN "Press SPACE BAR to continue" ANSI_COLOR_RESET;

void showInfoDialog(const char title[], const char text[])
{
	struct _BoxOptions options = {
		.title = title,
			.color = ANSI_COLOR_YELLOW,
			.paddingX = DIALOG_PADDING_X,
			.paddingY = DIALOG_PADDING_Y
	};
	struct WrapLine* lines = wrapBox(text, DIALOG_WIDTH, &options);
	lines = addNewline(lines);
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

void showStoryDialog(const char title[], const char text[])
{
	drawBox(text, DIALOG_WIDTH, BORDER_SINGLE, &(struct _BoxOptions){
		.title = title,
			.color = ANSI_COLOR_YELLOW,
			.paddingX = DIALOG_PADDING_X,
			.paddingY = DIALOG_PADDING_Y
	});
}

enum ConfirmationDialogReturn showConfirmationDialog(const char* text)
{
	Coord capture;
	drawBox(text, DIALOG_WIDTH, BORDER_DOUBLE, &(struct _BoxOptions){
		.color = ANSI_COLOR_RED,
			.paddingX = DIALOG_PADDING_X,
			.paddingY = DIALOG_PADDING_Y,
			.captures = &capture
	});
	setCursorPos(capture.x + 1 + DIALOG_PADDING_X, capture.y + 1 + DIALOG_PADDING_Y);
	fflush(stdout);

	return getBooleanInput(NULL);
}

void showPromptDialog(const char text[], char* buffer, short buffer_size)
{
	clearStdout();
	const size_t len = strlen(text);
	char* _text = malloc(len + 3);
	assert(_text);
	memcpy(_text, text, len);
	_text[len] = ' ';
	_text[len + 1] = CONTROL_CHAR;
	_text[len + 2] = 0;

	Coord capture;
	drawBox(_text, DIALOG_WIDTH, BORDER_DOUBLE, &(struct _BoxOptions){
		.color = ANSI_COLOR_YELLOW,
			.captures = &capture
	});
	setCursorPos(capture.x + 1, capture.y + 1);
	putsn(ANSI_CURSOR_SHOW);
	fflush(stdout);
	free(_text);

	getStringInput(buffer, 1, buffer_size, NULL);
}

void showErrorDialog(const char* context)
{
	char buffer[256];
	sprintf_s(buffer, sizeof(buffer), "%s: %s", context, strerror(errno));
	showInfoDialog("! Error !", buffer);
}

void putBlock(const char* text, byte x, byte y)
{
	char* _text = strdup(text);
	char* line;
	byte i = 0;

	line = strtok(_text, "\n");
	while (line != 0) {
		setCursorPos(x, y + i++);
		putsn(line);
		line = strtok(0, "\n");
	}
	fflush(stdout);
	free(_text);
}

void putBlockWL(struct WrapLine* lines, byte x, byte y, byte width)
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

		case WRAPLINEKIND_CENTER:
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

void putBlockWLFill(struct WrapLine* lines, byte x, byte y, byte width)
{
	for (byte i = 0; i < (byte)cvector_size(lines); i++)
	{
		setCursorPos(x, y + i);
		byte p;

		switch (lines[i].kind)
		{
		case WRAPLINEKIND_NONE:
			putsn(lines[i].text);
			break;

		case WRAPLINEKIND_LTR:
			putsn(lines[i].text);
			p = width - lines[i].client_length;
			while (p--) putchar(' ');
			break;

		case WRAPLINEKIND_CENTER:
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

	cvector_free(lines);
}

void indentLines(struct WrapLine* begin, struct WrapLine* end, const byte amount)
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

struct WrapLine* textToLines(const char* text)
{
	struct WrapLine* lines = NULL;
	cvector_init(lines, 0, NULL);
	return textToLinesWL(lines, text);
}

struct WrapLine* textToLinesWL(struct WrapLine* lines, const char* text)
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

struct WrapLine* addBar(struct WrapLine* lines)
{
	return addLine(lines, ANSI_COLOR_CYAN"컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴", WRAPLINEKIND_NONE);
}


#include <ctype.h>
#include "libs/cvector.h"

#include "store.h"
#include "tui.h"
#include "input.h"
#include "utils.h"
#include "state.h"
#include "store_data.h"

float total_bill = 0;

static struct WrapLine* showStoreCategory(struct WrapLine* lines, const struct StoreCategory* categories, byte* index)
{
	char text[16] = { '1' + *index, '.', ' ' };
	strcpy(text + 3, categories[*index].name);
	char text2[8];
	sprintf(text2, "$%.2f", categories[(*index)++].spent);
	return justifyLineWL(lines, text, text2, DIALOG_CONTENT_WIDTH);
}

static void showStoreCategoryMenu(struct StoreCategory* category, byte index, Coord input_pos)
{
	struct StoreItem* item = category->items;
	const struct StoreItem* items_end = &category->items[category->items_count];
	float last_total_bill = total_bill - category->spent;
	category->spent = 0;

	const char question_start[] = "\n\nHow many "CONTROL_CHAR_STR;
	const char question_end[] = "? "CONTROL_CHAR_STR;
	const size_t len1 = strlen(category->desciption),
		len2 = strlen(item->name);
	char* text = malloc(len1 + len2 + sizeof(question_start) + sizeof(question_end));
	assert(text);

	text[0] = '\n';
	size_t pos = 1;
	memcpy(text + pos, category->desciption, len1);
	pos += len1;
	memcpy(text + pos, question_start, sizeof(question_start) - 1);
	pos += sizeof(question_start) - 1;
	memcpy(text + pos, item->name, len2);
	pos += len2;
	memcpy(text + pos, question_end, sizeof(question_end));

	Coord captures[2];
	struct WrapLine* lines = wrapText(text, DIALOG_CONTENT_WIDTH, &(struct _WrapLineOptions){
		.captures = &captures[0]
	});
	free(text);

	lines = addNewline(lines);
	lines = textToLinesWL(lines, category->image);
	lines = addNewline(lines);
	char text2[24];
	sprintf(text2, "Bill so far: $%.2f", total_bill);
	lines = addLine(lines, text2, WRAPLINEKIND_CENTER);

	putsn(ANSI_SB_ALT);
	clearStdout();
	drawBoxWL(lines, DIALOG_WIDTH, BORDER_DOUBLE, &(struct _BoxOptions){
		.title = "Matt's General Store",
			.color = ANSI_COLOR_BLUE,
			.paddingX = DIALOG_PADDING_X
	});
	setCursorPos(captures[1].x + DIALOG_PADDING_X + 1, captures[1].y + DIALOG_PADDING_Y);

	item->amount = getNumber(item->min, item->max, 1, NULL);
	category->spent += item->price * item->amount;

	while (++item < items_end)
	{
		strcpy(text2, item->name);
		strcat(text2, question_end);
		struct WrapLine* lines = wrapText(text2, DIALOG_CONTENT_WIDTH, &(struct _WrapLineOptions){
			.captures = &captures[1]
		});
		putBlockWL(lines, captures[0].x + DIALOG_PADDING_X + 1, captures[0].y + DIALOG_PADDING_Y, 0);
		setCursorPos(captures[1].x + captures[0].x + DIALOG_PADDING_X + 1, captures[1].y + captures[0].y + DIALOG_PADDING_Y);

		item->amount = getNumber(item->min, item->max, 1, NULL);
		category->spent += item->price * item->amount;
	}

	putsn(ANSI_SB_MAIN);
	// redraw
	sprintf(text2, "   $%.2f", category->spent);
	setCursorPos(5 + DIALOG_CONTENT_WIDTH - (int)strlen(text2), 5 + index);
	putsn(text2);

	total_bill = last_total_bill + category->spent;
	sprintf(text2, "    Total bill: $%.2f", total_bill);
	setCursorPos(5 + DIALOG_CONTENT_WIDTH - (int)strlen(text2), 6 + _countof(STORE_MATT_CATEGORIES));
	putsn(text2);

	workStore(input_pos);
}

static void drawChoiceStore(byte index, bool selected)
{
	setCursorPos(5, 5 + index);
	if (selected) putsn(ANSI_SELECTED);
	putchar('1' + index);
	putsn(". ");
	putsn(STORE_MATT_CATEGORIES[index].name);
	if (selected) putsn(ANSI_COLOR_RESET);
}

static void showAlert(char text[])
{
	struct WrapLine* lines = wrapText(text, DIALOG_CONTENT_WIDTH, NULL);

	addStaticLine(lines, "Press SPACE BAR to continue", WRAPLINEKIND_CENTER);

	putBlockWLFill(lines, 5, 8 + _countof(STORE_MATT_CATEGORIES), DIALOG_CONTENT_WIDTH);
	putsn(ANSI_CURSOR_HIDE);
	waitForKey(' ');

	lines = NULL;
	cvector_init(lines, 0, 0);
	char buffer[32];
	sprintf(buffer, "Amount you have: $%.2f", money);
	lines = addLine(lines, buffer, WRAPLINEKIND_RTL);
	lines = addNewline(lines);

	lines = wrapText("Which item would you like to buy?\n\n", DIALOG_CONTENT_WIDTH - INDENT_SIZE, &(struct _WrapLineOptions){
		.lines = lines
	});
	addStaticLine(lines, "Press SPACE BAR to leave store", WRAPLINEKIND_CENTER);
	putBlockWLFill(lines, 5, 8 + _countof(STORE_MATT_CATEGORIES), DIALOG_CONTENT_WIDTH);
	putsn(ANSI_CURSOR_SHOW);
}

static enum QKeyCallbackReturn storeInputCallback(unsigned key, enum QKeyType type, va_list args)
{
	char* cur_pos = va_arg(args, char*);
	Coord end = va_arg(args, Coord);
	va_end(args);

	if (type == QKEY_TYPE_ARROW)
	{
		if (key == QARROW_DOWN && (short)*cur_pos + (char)1 != _countof(STORE_MATT_CATEGORIES))
		{
			if (*cur_pos != -1)
				drawChoiceStore(*cur_pos, 0);
			else
				putsn(ANSI_CURSOR_HIDE);

			drawChoiceStore(++*cur_pos, 1);
		}
		else if (key == QARROW_UP && *cur_pos != 0)
		{
			if (*cur_pos != -1)
				drawChoiceStore(*cur_pos, 0);
			else
			{
				*cur_pos = _countof(STORE_MATT_CATEGORIES);
				putsn(ANSI_CURSOR_HIDE);
			}
			drawChoiceStore(--*cur_pos, 1);
		}
	}
	else
	{
		if (key == ' ')
		{
			if (*cur_pos != -1)
			{
				drawChoiceStore(*cur_pos, 0);
				*cur_pos = -1;
			}
			if (money < total_bill)
			{
				char text[116];
				sprintf(text, "Okay, that comes to a total of $%.2f, but I see you only have $%.2f. We'd better go over the list again.\n\n", total_bill, money);
				showAlert(text);
				setCursorPos(end.x, end.y);
				return QKEY_CALLBACK_RETURN_IGNORE;
			}
			// oxen
			else if (STORE_MATT_CATEGORIES[0].items[0].amount == 0)
			{
				showAlert("Don't forget, you'll need oxen to pull your wagon.\n\n\n\n");
				setCursorPos(end.x, end.y);
				return QKEY_CALLBACK_RETURN_IGNORE;
			}
			else
			{
				money -= total_bill;

				oxen = STORE_MATT_CATEGORIES[0].items[0].amount * 2;
				food = STORE_MATT_CATEGORIES[1].items[0].amount + STORE_MATT_CATEGORIES[1].items[1].amount;
				clothing_sets = STORE_MATT_CATEGORIES[2].items[0].amount;
				bullets = STORE_MATT_CATEGORIES[3].items[0].amount * 20;
				wagon_axles = STORE_MATT_CATEGORIES[4].items[0].amount;
				wagon_wheels = STORE_MATT_CATEGORIES[4].items[1].amount;
				wagon_torques = STORE_MATT_CATEGORIES[4].items[2].amount;

				return QKEY_CALLBACK_RETURN_END;
			}
		}
		if (key == '\r' && *cur_pos != -1)
		{
			drawChoiceStore(*cur_pos, 0);
			putsn(ANSI_CURSOR_SHOW);
			showStoreCategoryMenu(&STORE_MATT_CATEGORIES[*cur_pos], *cur_pos, end);
			return QKEY_CALLBACK_RETURN_END;
		}
		else if (key == 27 && *cur_pos > -1)
		{
			drawChoiceStore(*cur_pos, 0);
			*cur_pos = -1;
			setCursorPos(end.x, end.y);
			putsn(ANSI_CURSOR_SHOW);
		}
	}
	return *cur_pos == -1 ? QKEY_CALLBACK_RETURN_NORMAL : QKEY_CALLBACK_RETURN_IGNORE;
}

Coord drawStore(void)
{
	struct WrapLine* lines = NULL;
	cvector_init(lines, 0, NULL);
	lines = addLine(lines, &location[0], WRAPLINEKIND_CENTER);
	lines = addNewline(lines);
	char date[16];
	memcpy(date, MONTHS[month], sizeof(MONTHS[0]));
	strcat(date, " 1, 1868");
	lines = addLine(lines, date, WRAPLINEKIND_RTL);
	lines = addBar(lines);

	byte i = 0;
	total_bill = 0.f;
	while (i != _countof(STORE_MATT_CATEGORIES))
	{
		total_bill += STORE_MATT_CATEGORIES[i].spent;
		lines = showStoreCategory(lines, &STORE_MATT_CATEGORIES[0], &i);
	}
	lines = addBar(lines);
	char text[32];
	sprintf(text, "Total bill: $%.2f", total_bill);
	lines = addLine(lines, text, WRAPLINEKIND_RTL);
	lines = addNewline(lines);
	sprintf(text, "Amount you have: $%.2f", money);
	lines = addLine(lines, text, WRAPLINEKIND_RTL);
	lines = addNewline(lines);

	Coord capture;
	lines = wrapText("Which item would you like to buy? "CONTROL_CHAR_STR"\n\n", DIALOG_CONTENT_WIDTH - INDENT_SIZE, &(struct _WrapLineOptions){
		.captures = &capture,
			.lines = lines
	});
	addStaticLine(lines, "Press SPACE BAR to leave store", WRAPLINEKIND_CENTER);

	clearStdout();
	drawBoxWL(lines, DIALOG_WIDTH, BORDER_DOUBLE, &(struct _BoxOptions){
		.title = "Matt's General Store",
			.paddingX = DIALOG_PADDING_X,
			.color = "\033[38;5;94m"
	});
	putsn(ANSI_CURSOR_SAVE);

	capture.x += DIALOG_PADDING_X + 1;
	++capture.y;
	return capture;
}

void workStore(Coord input_pos)
{
	setCursorPos(input_pos.x, input_pos.y);
	char cur_pos = -1;
	const int choice = vgetNumber(1, _countof(STORE_MATT_CATEGORIES), 1, &storeInputCallback, &cur_pos, input_pos) - 1;
	if (choice >= 0)
		showStoreCategoryMenu(&STORE_MATT_CATEGORIES[choice], choice, input_pos);
}
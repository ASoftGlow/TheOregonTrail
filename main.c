#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "libs/cvector.h"

#include "tui.h"
#include "main.h"
#include "utils.h"
#include "input.h"

const char dialog_txt[] = "You may:";
const char dialog_prompt[] = "What is your choice?";

enum Role role;
byte month;
struct WagonMember wagon_leader[NAME_SIZE + 1];
struct WagonMember wagon_members[WAGON_MEMBER_COUNT];
float money = 0.f;
int
bullets = 0,
clothing_sets = 0,
oxen = 0,
wagon_axles = 0,
wagon_wheels = 0,
wagon_torques = 0;
byte water = 255;
short food;

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

#define question_start "\n\nHow many "CONTROL_CHAR_STR
#define question_end   "? "CONTROL_CHAR_STR
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

#undef question_start
#undef question_end
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

static enum QKeyCallbackReturn storeInputCallback(unsigned key, enum QKeyType type, va_list args)
{
	if (type == QKEY_TYPE_NORMAL)
	{
		if (key == ' ')
		{
			if (money < total_bill)
			{
				char text[116];
				sprintf(text, "Okay, that comes to a total of $%.2f, but I see you only have $%.2f. We'd better go over the list again.\n\n", total_bill, money);
				struct WrapLine* lines = wrapText(text, DIALOG_CONTENT_WIDTH, NULL);
				addStaticLine(lines, "Press SPACE BAR to continue", WRAPLINEKIND_CENTER);

				putBlockWLFill(lines, 5, 8 + _countof(STORE_MATT_CATEGORIES), DIALOG_CONTENT_WIDTH);
				putsn(ANSI_CURSOR_HIDE);
				waitForKey(' ');

				lines = 0;
				cvector_init(lines, 0, 0);
				sprintf(text, "Amount you have: $%.2f", money);
				lines = addLine(lines, text, WRAPLINEKIND_RTL);
				lines = addNewline(lines);

				lines = wrapText("Which item would you like to buy?    \n\n", DIALOG_CONTENT_WIDTH - DIALOG_PADDING_X, &(struct _WrapLineOptions){
					.lines = lines
				});
				addStaticLine(lines, "Press SPACE BAR to leave store", WRAPLINEKIND_CENTER);
				putBlockWLFill(lines, 5, 8 + _countof(STORE_MATT_CATEGORIES), DIALOG_CONTENT_WIDTH);
				putsn(ANSI_CURSOR_SHOW);
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
	}
	return QKEY_CALLBACK_RETURN_NORMAL;
}

Coord drawStore(void)
{
	struct WrapLine* lines = 0;
	cvector_init(lines, 0, 0);
	addStaticLine(lines, "Independence, Missouri", WRAPLINEKIND_CENTER);
	lines = addNewline(lines);
	char date[16];
	memcpy(date, months[month], sizeof(months[0]));
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
	lines = wrapText("Which item would you like to buy? "CONTROL_CHAR_STR"\n\n", DIALOG_CONTENT_WIDTH - DIALOG_PADDING_X, &(struct _WrapLineOptions){
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
	const int choice = vgetNumber(1, _countof(STORE_MATT_CATEGORIES), 1, &storeInputCallback, &total_bill) - 1;
	if (choice >= 0)
		showStoreCategoryMenu(&STORE_MATT_CATEGORIES[choice], choice, input_pos);
}

void showStore(void)
{
	total_bill = 0;
	for (byte i = 0; i < _countof(STORE_MATT_CATEGORIES); i++)
	{
		total_bill += STORE_MATT_CATEGORIES[i].spent;
	}

	const Coord capture = drawStore();
	putsn(ANSI_CURSOR_SHOW);
	workStore(capture);
}

static declare_choice_callback_g(month)
{
	month = index;

	char text[256];

	sprintf(text, "Before leaving Independence you should buy equipment and supplies. You have $%.2f in cash, but you don't have to spend it all now.", money);
	showInfoDialog(0, text);
	showInfoDialog(0, "You can buy whatever you need at Matt's General Store.");
#define TAB "   "
#define matt_greeting "Hello, I'm Matt. So you're going to Oregon! I can fix you up with what you need:\n\n"
	showInfoDialog("Meet Matt", matt_greeting TAB"- a team of oxen to pull\n"TAB"  your wagon\n\n"TAB"- clothing for both\n"TAB"  summer and winter");
	showInfoDialog("Meet Matt", matt_greeting TAB"- plenty of food for the\n"TAB"  trip\n\n"TAB"- ammunition for your\n"TAB"  rifles\n\n"TAB"- spare parts for your\n"TAB"  wagon");
#undef matt_greeting
	showStore();
}

static declare_choice_callback(month_advice)
{
	showInfoDialog("Month Info", "You attend a public meeting held for \"folks with the California - Oregon fever.\" You're told:\n\nIf you leave too early, there won't be any grass for your oxen to eat. If you leave too late, you may not get to Oregon before winter comes. If you leave at just the right time, there will be green grass and the weather will still be cool.");
	showMonth();
}

const struct ChoiceDialogChoice month_choices[] = {
	{"March"},
	{"April"},
	{"May"},
	{"June"},
	{"July"},
	{"Ask for advice",.callback = choice_callback(month_advice)}
};
void showMonth(void)
{
	const char text[] = "It is 1848. Your jumping off place for Oregon is Independence, Missouri. You must decide which month to leave Independence.";

	showChoiceDialog(text, dialog_prompt, month_choices, _countof(month_choices), &(struct _DialogOptions){
		.callback = choice_callback(month)
	});
}

static declare_choice_callback(role_learn)
{
	showInfoDialog("Role Info", "Traveling to Oregon isn't easy! But if you're a banker, you'll have more money for supplies and services than a carpenter or a farmer.\n\nHowever, the harder you have to try, the more points you deserve! Therefore, the farmer earns the greatest number of points and the banker earns the least.");
	showRole();
}

static declare_choice_callback_g(role)
{
	role = index;
	money = (float[]){ 1600.f, 800.f, 400.f } [index] ;

	clearStdout();
	drawBox(&("What is the first name of the wagon leader? " ANSI_CURSOR_SAVE)[0], DIALOG_WIDTH, BORDER_DOUBLE, &(struct _BoxOptions){
		.height = 8,
			.color = ANSI_COLOR_YELLOW
	});
	putsn(ANSI_CURSOR_RESTORE ANSI_CURSOR_SHOW);

	getString(&wagon_leader->name[0], 1, NAME_SIZE, 0);

	for (int i = 0; i < WAGON_MEMBER_COUNT; i++)
	{
		strcpy(wagon_members[i].name, getRandomName());
	}

	showMonth();
}

const struct ChoiceDialogChoice role_choices[] = {
	{"Be a banker from Boston"},
	{"Be a carpenter from Ohio"},
	{"Be a farmer from Illinois"},
	{"Find out the differences between these choices",.callback = choice_callback(role_learn)}
};
void showRole(void)
{
	showChoiceDialog("Many kinds of people made the trip to Oregon.\n\nYou may:", dialog_prompt, role_choices, _countof(role_choices), &(struct _DialogOptions){
		.callback = choice_callback(role)
	});
}

static declare_choice_callback(main_start)
{
	showRole();
}

static declare_choice_callback(main_learn)
{
	showInfoDialog("Oregon Trail Info", "idk lol\n\n\n");
	showMain();
}

static declare_choice_callback(main_top)
{

}

static declare_choice_callback(main_exit)
{
	putsn(ANSI_CURSOR_RESTORE ANSI_COLOR_RESET ANSI_CURSOR_SHOW);
}

const struct ChoiceDialogChoice main_choices[] = {
	{ANSI_COLOR_CYAN "Travel the trail" ANSI_COLOR_RESET,.callback = choice_callback(main_start)},
	{"Learn about the trail",.callback = choice_callback(main_learn)},
	{"See the Oregon Top Ten",.callback = choice_callback(main_top)},
	//{"Choose Management Options"},
	{"Exit",.callback = choice_callback(main_exit)}
};
void showMain(void)
{
	showChoiceDialog(dialog_txt, dialog_prompt, main_choices, _countof(main_choices), &(struct _DialogOptions){
		.title = "Welcome to Oregon Trail"
	});
}

int main(void)
{
#ifdef _WIN32
	setupConsoleWIN();
#endif

	putsn(ANSI_CURSOR_STYLE_UNDERLINE ANSI_CURSOR_SHOW ANSI_WINDOW_TITLE("Oregon Trail") ANSI_WINDOW_SIZE("42", ""));
	showMain();
}

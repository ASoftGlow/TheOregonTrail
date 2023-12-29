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
const char months[][8] = {
	"March",
	"April",
	"May",
	"June",
	"July"
};

enum Role role;
int month;
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

static struct WrapLine* showStoreCategory(struct WrapLine* lines, const struct StoreCategory* categories, byte* index)
{
	char text[16] = { '1' + *index, '.', ' ' };
	strcpy(text + 3, categories[*index].name);
	char text2[8];
	sprintf(text2, "$%.2f", categories[(*index)++].spent);
	return justifyLineWL(lines, text, text2, DIALOG_CONTENT_WIDTH);
}

static struct WrapLine* addBar(struct WrapLine* lines)
{
	return addLine(lines, ANSI_COLOR_CYAN"컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴", WRAPLINEKIND_NONE);
}

static void showStoreCategoryMenu(struct StoreCategory* category)
{
	struct StoreItem* item = category->items;
	const struct StoreItem* items_end = &category->items[category->items_count];
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
	sprintf(text2, "Bill so far: $%.2f", 99.f);
	lines = addLine(lines, text2, WRAPLINEKIND_CENTER);

	clearStdout();
	drawBoxWL(lines, DIALOG_WIDTH, BORDER_DOUBLE, &(struct _BoxOptions){
		.title = "Matt's General Store",
			.color = ANSI_COLOR_BLUE,
			.paddingX = DIALOG_PADDING_X
	});
	setCursorPos(captures[1].x + DIALOG_PADDING_X + 1, captures[1].y + DIALOG_PADDING_Y);

	item->amount = getNumber(item->min, item->max, NULL);
	category->spent += item->price * item->amount;

	while (++item < items_end)
	{
		strcpy(text2, item->name);
		strcat(text2, question_end);
		struct WrapLine* lines = wrapText(text2, DIALOG_CONTENT_WIDTH, &(struct _WrapLineOptions){
			.captures = &captures[1]
		});
		putsn("\b ");
		putBlockWL(lines, captures[0].x + DIALOG_PADDING_X + 1, captures[0].y + DIALOG_PADDING_Y);
		setCursorPos(captures[1].x + captures[0].x + DIALOG_PADDING_X + 1, captures[1].y + captures[0].y + DIALOG_PADDING_Y);

		item->amount = getNumber(item->min, item->max, NULL);
		category->spent += item->price * item->amount;
	}

#undef question_start
#undef question_end
	showStore();
}

struct StoreCategory STORE_MATT_CATEGORIES[] = {
	{
		.name = "Oxen",
		.desciption = "There are 2 oxen in a yoke; I recommend at least 3 yoke. I charge $40 a yoke.",
		.image = "o-uu,",
		.items = &(struct StoreItem[]) {
			{.name = "yokes", .price = 10.f, .min = 1, .max = 9}
		},
		.items_count = 1
	},
	{
		.name = "Food",
		.desciption = "I recommend you take at least 200 pounds of food for each person in your family. I see that you have 5 people in all. You'll need flour, sugar, bacon, and coffee. My price is 20 cents a pound.",
		.image = "()O=-+\n    _\n   \\ \\",
		.items = &(struct StoreItem[]) {
			{.name = "pounds of bacon", .price = .2f},
			{.name = "pounds of flour", .price = .2f}
		},
		.items_count = 2
	},
	{
		.name = "Clothing",
		.desciption = "You'll need warm clothing in the mountains. I recommend taking at least 2 sets of clothes per person, $10.00 each.",
		.image = "__\n||",
		.items = &(struct StoreItem[]) {
			{.name = "sets of clothes", .price = 10.f}
		},
		.items_count = 1
	},
	{
		.name = "Ammunition",
		.desciption = "I sell ammunition in boxes of 20 bullets. Each box costs $2.00.",
		.image = "gun",
		.items = &(struct StoreItem[]) {
			{.name = "boxes", .price = 2.f}
		},
		.items_count = 1
	},
	{
		.name = "Spare parts",
		.desciption = "It's a good idea to have a few spare parts for your wagon. Here are the prices:\n\n    axle - $10\n   wheel - $10\n  tongue - $10",
		.image = "O _",
		.items = &(struct StoreItem[]) {
			{.name = "axles", .price = 10.f},
			{.name = "wheels", .price = 10.f},
			{.name = "tongues", .price = 10.f}
		},
		.items_count = 3
	}
};

static enum QKeyCallbackReturn storeInputCallback(int key, enum QKeyType type, va_list args)
{
	float total_bill = va_arg(args, float);

	if (type == QKEY_TYPE_NORMAL)
	{
		if (key == ' ')
		{
			if (money < total_bill)
			{

			}
			else
			{
				return QKEY_CALLBACK_RETURN_END;
			}
		}
	}
	return QKEY_CALLBACK_RETURN_NORMAL;
}

void showStore(void)
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

	float total_bill = 0;
	byte i = 0;
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
	lines = wrapText("Which item would you like to buy? "CONTROL_CHAR_STR"\n\nPress SPACE BAR to leave store", DIALOG_WIDTH - DIALOG_PADDING_X * 3, &(struct _WrapLineOptions){
		.captures = &capture,
			.lines = lines
	});

	clearStdout();
	drawBoxWL(lines, DIALOG_WIDTH, BORDER_DOUBLE, &(struct _BoxOptions){
		.title = "Matt's General Store",
			.paddingX = DIALOG_PADDING_X,
			.color = "\033[38;5;94m"
	});

	setCursorPos(capture.x + DIALOG_PADDING_X + 1, capture.y + 1);
	putsn(ANSI_CURSOR_SHOW);
	int choice = vgetNumber(1, _countof(STORE_MATT_CATEGORIES), &storeInputCallback, total_bill) - 1;
	showStoreCategoryMenu(&STORE_MATT_CATEGORIES[choice]);
}

choice_callback_g(month)
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

choice_callback(month_advice)
{
	showInfoDialog("Month Info", "You attend a public meeting held for \"folks with the California - Oregon fever.\" You're told:\n\nIf you leave too early, there won't be any grass for your oxen to eat. If you leave too late, you may not get to Oregon before winter comes. If you leave at just the right time, there will be green grass and the weather will still be cool.");
	showMonth();
}

void showMonth(void)
{
	const char text[] = "It is 1848. Your jumping off place for Oregon is Independence, Missouri. You must decide which month to leave Independence.";
	const struct ChoiceDialogChoice choices[] = {
		{"March"},
		{"April"},
		{"May"},
		{"June"},
		{"July"},
		{"Ask for advice",.callback = &menu_month_advice}
	};

	showChoiceDialog(text, dialog_prompt, choices, _countof(choices), &(struct _DialogOptions){
		.callback = &menu_month
	});
}

choice_callback(role_learn)
{
	showInfoDialog("Role Info", "Traveling to Oregon isn't easy! But if you're a banker, you'll have more money for supplies and services than a carpenter or a farmer.\n\nHowever, the harder you have to try, the more points you deserve! Therefore, the farmer earns the greatest number of points and the banker earns the least.");
	showRole();
}

choice_callback_g(role)
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

void showRole(void)
{
	const struct ChoiceDialogChoice choices[] = {
		{"Be a banker from Boston"},
		{"Be a carpenter from Ohio"},
		{"Be a farmer from Illinois"},
		{"Find out the differences between these choices",.callback = &menu_role_learn}
	};

	showChoiceDialog("Many kinds of people made the trip to Oregon.\n\nYou may:", dialog_prompt, choices, _countof(choices), &(struct _DialogOptions){
		.callback = &menu_role
	});
}

choice_callback(main_start)
{
	showRole();
}

choice_callback(main_learn)
{
	showInfoDialog("Oregon Trail Info", "idk lol\n\n\n");
	showMain();
}

choice_callback(main_top)
{

}

choice_callback(main_exit)
{
	putsn(ANSI_CURSOR_RESTORE ANSI_COLOR_RESET ANSI_CURSOR_SHOW);
	exit(0);
}

void showMain(void)
{
	const struct ChoiceDialogChoice choices[] = {
		{ANSI_COLOR_CYAN "Travel the trail" ANSI_COLOR_RESET,.callback = &menu_main_start},
		{"Learn about the trail",.callback = &menu_main_learn},
		{"See the Oregon Top Ten",.callback = &menu_main_top},
		//{"Choose Management Options"},
		{"Exit",.callback = &menu_main_exit}
	};

	showChoiceDialog(dialog_txt, dialog_prompt, choices, _countof(choices), &(struct _DialogOptions){
		.title = "Welcome to Oregon Trail"
	});
}

int main(void)
{
#ifdef _WIN32
	enableANSICodes();
#endif

	putsn(ANSI_CURSOR_STYLE_UNDERLINE ANSI_CURSOR_SHOW);
	showStore();
}

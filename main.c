#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#ifdef _WIN32
#include <Windows.h>
#endif

#include "constants.h"
#include "tui.h"
#include "utils.h"
#include "input.c"

const char dialog_txt[] = "You may:";
const char dialog_prompt[] = "What is your choice?";
const char dialog_leader_name[] = "What is the first name of the wagon leader? " ANSI_CURSOR_SAVE;

#define WAGON_MEMBER_COUNT 4
#define NAME_SIZE 10
const char WAGON_MEMBER_NAMES[][NAME_SIZE] = { "John", "Sara", "Henry", "Jed" };

enum Role role;
int month;
char wagon_leader[NAME_SIZE + 1];
char wagon_member[WAGON_MEMBER_COUNT][NAME_SIZE + 1];

enum Role {
	BANKER,
	CARPENTER,
	FARMER
};

void show_main(void);
void show_month(void);
void show_role(void);


static choice_callback_g(month)
{
	month = index;

	showInfoDialog(0, "Before leaving Independence you should buy equipment and supplies. You have $1600.00 in cash, but you don't have to spend it all now.");
}

static choice_callback(month_advice)
{
	showInfoDialog("Month Info", "You attend a public meeting held for \"folks with the California - Oregon fever.\" You're told:\n\nIf you leave too early, there won't be any grass for your oxen to eat. If you leave too late, you may not get to Oregon before winter comes. If you leave at just the right time, there will be green grass and the weather will still be cool.");
	show_month();
}

const char menu_month_text[] = "It is 1848. Your jumping off place for Oregon is Independence, Missouri. You must decide which month to leave Independence.";
const struct ChoiceDialogChoice menu_month_choices[] = {
	{"March"},
	{"April"},
	{"May"},
	{"June"},
	{"July"},
	{"Ask for advice",.callback = &menu_month_advice}
};
static void show_month(void)
{
	showChoiceDialog(0, menu_month_text, dialog_prompt, menu_month_choices, _countof(menu_month_choices), &menu_month, 0);
}

static choice_callback(role_learn)
{
	showInfoDialog("Role Info", "Traveling to Oregon isn't easy! But if you're a banker, you'll have more money for supplies and services than a carpenter or a farmer.\n\nHowever, the harder you have to try, the more points you deserve! Therefore, the farmer earns the greatest number of points and the banker earns the least.");
	show_role();
}

static choice_callback_g(role)
{
	role = index;

	clear_stdout();
	drawBox(DIALOG_WIDTH, 8, dialog_leader_name, 0, BORDER_DOUBLE, ANSI_COLOR_YELLOW);
	puts_n(ANSI_CURSOR_SHOW ANSI_CURSOR_RESTORE);

	fputc('t', stdin);
	scanf_s("%10[a-zA-Z]s", wagon_leader, (unsigned)(NAME_SIZE + 1));

	//WAGON_MEMBER_NAMES[rand() % _countof(WAGON_MEMBER_NAMES)];

	puts_n(ANSI_CURSOR_HIDE);

	show_month();
}

const char menu_role_text[] = "Many kinds of people made the trip to Oregon.\n\nYou may:";
const struct ChoiceDialogChoice menu_role_choices[] = {
	{"Be a banker from Boston"},
	{"Be a carpenter from Ohio"},
	{"Be a farmer from Illinois"},
	{"Find out the differences between these choices",.callback = &menu_role_learn}
};
void show_role(void)
{
	showChoiceDialog(0, menu_role_text, dialog_prompt, menu_role_choices, _countof(menu_role_choices), &menu_role, 0);
}

static choice_callback(main_start)
{
	show_role();
}

static choice_callback(main_learn)
{
	showInfoDialog("Oregon Trail Info", "idk lol");
	show_main();
}

static choice_callback(main_top)
{

}

static choice_callback(main_exit)
{
	exit(0);
}

const struct ChoiceDialogChoice menu_main_choices[] = {
	{ANSI_COLOR_CYAN "Travel the trail" ANSI_COLOR_RESET,.callback = &menu_main_start},
	{"Learn about the trail",.callback = &menu_main_learn},
	{"See the Oregon Top Ten",.callback = &menu_main_top},
	//{"Choose Management Options"},
	{"Exit",.callback = &menu_main_exit}
};
void show_main(void)
{
	showChoiceDialog("Welcome to Oregon Trail", dialog_txt, dialog_prompt, menu_main_choices, _countof(menu_main_choices), 0, 0);
}

int main(void)
{
#ifdef _WIN32
	// enable ANSI escape codes
	HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
	DWORD dwMode = 0;
	GetConsoleMode(hOut, &dwMode);
	dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
	SetConsoleMode(hOut, dwMode);
#endif

	puts_n(ANSI_CURSOR_HIDE);
	show_main();

	//drawBox(21, 5, text, title, BORDER_DOUBLE, ANSI_COLOR_YELLOW);
}


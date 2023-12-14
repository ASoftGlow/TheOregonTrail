#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#ifdef _WIN32
#include <Windows.h>
#else
typedef unsigned char byte;
#endif


#include "constants.h"
#include "tui.h"
#include "utils.h"
#include "input.h"

const char dialog_txt[] = "You may:";
const char dialog_prompt[] = "What is your choice?";
const char dialog_leader_name[] = "What is the first name of the wagon leader? " ANSI_CURSOR_SAVE;

#define WAGON_MEMBER_COUNT 4
#define NAME_SIZE 10
const char WAGON_MEMBER_NAMES[][NAME_SIZE] = { "John", "Sara", "Henry", "Jed" };

enum WagonMemberHealth
{
	HEALTH_GOOD,
	HEALTH_FAIR,
	HEALTH_POOR,
	HEALTH_VERY_POOR
};

enum Disease
{
	DISEASE_NONE
};
const byte disease_durations[] = { 0 };
const byte disease_chances[] = { 0 };

enum Injury
{
	INJURY_NONE,
	INJURY_BROKEN_ARM,
	INJURY_BROKEN_LEG
};
const byte injury_durations[] = { 20, 30 };
const byte injury_chances[] = { 20, 30 };

struct WagonMember
{
	char name[NAME_SIZE + 1];
	enum WagonMemberHealth health;
	enum Disease disease;
	byte disease_duration;
	enum Injury injury;
	byte injury_duration;
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

enum Role
{
	ROLE_BANKER,
	ROLE_CARPENTER,
	ROLE_FARMER
};

void show_main(void);
void show_month(void);
void show_role(void);
void show_store(void);

static const char* getRandomName(void)
{
	return WAGON_MEMBER_NAMES[rand() % _countof(WAGON_MEMBER_NAMES)];
}

void show_store(void)
{

}

static choice_callback_g(month)
{
	month = index;

	char text[256];

	sprintf(text, "Before leaving Independence you should buy equipment and supplies. You have $%.2f in cash, but you don't have to spend it all now.", money);
	showInfoDialog(0, text);
	showInfoDialog(0, "You can buy whatever you need at Matt's General Store.");
	showInfoDialog("Meet Matt", "Hello, I'm Matt. So you're going to Oregon! I can fix you up with what you need:\n\n- a team of oxen to pull\n  your wagon\n\n- clothing for summer and winter");
	show_store();
}

static choice_callback(month_advice)
{
	showInfoDialog("Month Info", "You attend a public meeting held for \"folks with the California - Oregon fever.\" You're told:\n\nIf you leave too early, there won't be any grass for your oxen to eat. If you leave too late, you may not get to Oregon before winter comes. If you leave at just the right time, there will be green grass and the weather will still be cool.");
	show_month();
}

void show_month(void)
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

	showChoiceDialog(0, text, dialog_prompt, choices, _countof(choices), &menu_month, 0);
}

static choice_callback(role_learn)
{
	showInfoDialog("Role Info", "Traveling to Oregon isn't easy! But if you're a banker, you'll have more money for supplies and services than a carpenter or a farmer.\n\nHowever, the harder you have to try, the more points you deserve! Therefore, the farmer earns the greatest number of points and the banker earns the least.");
	show_role();
}

static choice_callback_g(role)
{
	role = index;
	money = (float[]){ 1600.f, 800.f, 400.f } [index] ;

	clear_stdout();
	drawBox(DIALOG_WIDTH, 8, dialog_leader_name, 0, BORDER_DOUBLE, ANSI_COLOR_YELLOW, 0, 0);
	puts_n(ANSI_CURSOR_SHOW ANSI_CURSOR_RESTORE);

	fputc('t', stdin);
	scanf_s("%10[a-zA-Z]s", wagon_leader->name, (unsigned)(NAME_SIZE + 1));

	for (int i = 0; i < WAGON_MEMBER_COUNT; i++)
	{
		strcpy(wagon_members[i].name, getRandomName());
	}

	puts_n(ANSI_CURSOR_HIDE);

	show_month();
}

void show_role(void)
{
	const char text[] = "Many kinds of people made the trip to Oregon.\n\nYou may:";
	const struct ChoiceDialogChoice choices[] = {
		{"Be a banker from Boston"},
		{"Be a carpenter from Ohio"},
		{"Be a farmer from Illinois"},
		{"Find out the differences between these choices",.callback = &menu_role_learn}
	};

	showChoiceDialog(0, text, dialog_prompt, choices, _countof(choices), &menu_role, 0);
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
	puts_n(ANSI_CURSOR_SHOW);
	exit(0);
}

void show_main(void)
{
	const struct ChoiceDialogChoice choices[] = {
		{ANSI_COLOR_CYAN "Travel the trail" ANSI_COLOR_RESET,.callback = &menu_main_start},
		{"Learn about the trail",.callback = &menu_main_learn},
		{"See the Oregon Top Ten",.callback = &menu_main_top},
		//{"Choose Management Options"},
		{"Exit",.callback = &menu_main_exit}
	};

	showChoiceDialog("Welcome to Oregon Trail", dialog_txt, dialog_prompt, choices, _countof(choices), 0, 0);
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
	//getNumber(3, 8);
	
	puts_n(ANSI_CURSOR_SHOW);
	show_main();

	//drawBox(21, 5, text, title, BORDER_DOUBLE, ANSI_COLOR_YELLOW);
}


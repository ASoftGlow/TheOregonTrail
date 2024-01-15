#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "libs/cvector.h"

#include "base.h"
#include "static.h"
#include "tui.h"
#include "utils.h"
#include "input.h"
#include "state.h"
#include "store.h"
#include "map.h"

void showMainMenu(void);
void showMonth(void);
void showRole(void);
void showStore(void);
void showMain(void);

/**
 * @brief Computes the quick health based on each member's health
 * @return the string description
 */
static const char* getQuickHealth(void)
{
	// TODO
	return "good";
}

static declare_choice_callback(map)
{
	putsn(ANSI_SB_ALT);
	showMap();
	putsn(ANSI_SB_MAIN);
	//showMain();
}

const struct ChoiceDialogChoice SETTLEMENT_CHOICES[] = {
	{.name = "Continue on trail"},
	{.name = "Check supplies"},
	{.name = "Look at map", .callback=choice_callback(map)}
};

/**
 * @return the new pointer to <lines>
 */
static struct WrapLine* addQuickInfo(struct WrapLine* lines)
{
	lines = addBar(lines);

	// Weather
	char buffer[32] = ANSI_SELECTED"Weather: ";
	strcat(buffer, WEATHERS[state.weather]);
	lines = addLine(lines, buffer, WRAPLINEKIND_LTR);

	// Health
	strcpy(buffer, ANSI_SELECTED"Health: ");
	strcat(buffer, getQuickHealth());
	lines = addLine(lines, buffer, WRAPLINEKIND_LTR);

	// Pace
	strcpy(buffer, ANSI_SELECTED"Pace: ");
	strcat(buffer, PACES[state.pace]);
	lines = addLine(lines, buffer, WRAPLINEKIND_LTR);

	// Ration
	strcpy(buffer, ANSI_SELECTED"Rations: ");
	strcat(buffer, RATIONS[state.ration]);
	lines = addLine(lines, buffer, WRAPLINEKIND_LTR);

	lines = addBar(lines);
	return lines;
}

void showMain(void)
{
	struct WrapLine* lines = NULL;
	cvector_init(lines, 0, NULL);

	char date[16];
	memcpy(date, MONTHS[state.month], sizeof(MONTHS[0]));
	strcat(date, " 1, 1868");

	lines = addLine(lines, date, WRAPLINEKIND_CENTER);
	lines = addNewline(lines);

	lines = addQuickInfo(lines);
	addStaticLine(lines, "Work in progress from here on", 0);

	showChoiceDialogWL(lines, &SETTLEMENT_CHOICES[0], _countof(SETTLEMENT_CHOICES), &(struct _DialogOptions){
		.title = state.location,
			.noPaddingY = 1
	});
}

void showStore(void)
{
	// set location
	strcpy(state.location, "Independence, Missouri");

	const Coord capture = drawStore();
	putsn(ANSI_CURSOR_SHOW);
	fflush(stdout);
	workStore(capture);

	showInfoDialog("Parting with Matt", "Well then, you're ready to go. Good luck! You have a long and difficult journey ahead of you.");
	showMain();
}

static declare_choice_callback_g(month)
{
	state.month = index;

	char text[256];

	sprintf(text, "Before leaving Independence you should buy equipment and supplies. You have $%.2f in cash, but you don't have to spend it all now.", state.money);
	showInfoDialog(0, text);
	showInfoDialog(0, "You can buy whatever you need at Matt's General Store.");
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

	showChoiceDialog(text, month_choices, _countof(month_choices), &(struct _DialogOptions){
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
	state.role = index;
	// money per role
	state.money = (float[]){ 1600.f, 800.f, 400.f } [index] ;

	clearStdout();
	drawBox(&("What is the first name of the wagon leader? " ANSI_CURSOR_SAVE)[0], DIALOG_WIDTH, BORDER_DOUBLE, &(struct _BoxOptions){
		.height = 8,
			.color = ANSI_COLOR_YELLOW
	});
	putsn(ANSI_CURSOR_RESTORE ANSI_CURSOR_SHOW);
	fflush(stdout);

	getStringInput(&state.wagon_leader->name[0], 1, NAME_SIZE, 0);

	for (int i = 0; i < WAGON_MEMBER_COUNT; i++)
	{
		strcpy(state.wagon_members[i].name, getRandomName());
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
	showChoiceDialog("Many kinds of people made the trip to Oregon.\n\nYou may:", role_choices, _countof(role_choices), &(struct _DialogOptions){
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
	showMainMenu();
}

static declare_choice_callback(main_top)
{

}

static declare_choice_callback(main_exit)
{
	putsn(ANSI_CURSOR_RESTORE ANSI_COLOR_RESET ANSI_CURSOR_SHOW);
}

static declare_choice_callback(management)
{
	showInfoDialog("Compiled "__DATE__, "Recreating The Oregon Trail by MECC with only text.");
	showMainMenu();
}

const struct ChoiceDialogChoice main_choices[] = {
	{ANSI_COLOR_CYAN "Travel the trail" ANSI_COLOR_RESET,.callback = choice_callback(main_start)},
	{"Learn about the trail", .callback = choice_callback(main_learn)},
	{"See the Oregon Top Ten", .callback = choice_callback(main_top)},
	{"Choose Management Options", .callback = choice_callback(management)},
	{"Exit",.callback = choice_callback(main_exit)}
};
void showMainMenu(void)
{
	showChoiceDialog("You may:", main_choices, _countof(main_choices), &(struct _DialogOptions){
		.title = "Welcome to Oregon Trail"
	});
}

int main(void)
{
#ifdef _WIN32
	setupConsoleWIN();
#endif

	// make stdout fully buffered
	setvbuf(stdout, NULL, _IOFBF, (size_t)1 << 12);

	// style console
	putsn(ANSI_CURSOR_STYLE_UNDERLINE ANSI_CURSOR_SHOW ANSI_WINDOW_TITLE("Oregon Trail") ANSI_WINDOW_SIZE("42", "") ANSI_NO_WRAP);

	showMainMenu();
	//showStore();
	//showMap();
	/*strcpy(state.location, "Independence, Missouri");
	showMain();*/
}

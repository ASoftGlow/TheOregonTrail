#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include "cvector.h"
#include "nfd.h"

#include "main.h"
#include "base.h"
#include "static.h"
#include "tui.h"
#include "utils.h"
#include "input.h"
#include "state.h"
#include "store.h"
#include "map.h"
#include "settings.h"
#include "setup.h"

void showMainMenu(void);
void showMonth(void);
void showRole(void);
void showStore(void);
void showSavePrompt(void);

#define DEBUG_SAVE_PATH "../../resources/save.dat"

void showSavePrompt(void)
{
	switch (showConfirmationDialog("Would you like to save? "CONTROL_CHAR_STR))
	{
	case CONFIRMATION_DIALOG_QUIT:
	case CONFIRMATION_DIALOG_NO:
		return;

	case CONFIRMATION_DIALOG_YES:
#ifdef _DEBUG
		saveState(DEBUG_SAVE_PATH);
#else
		if (IS_TTY)
		{
			char path[FILENAME_MAX];
			clearStdout();
			puts("Enter a path:");
			fflush(stdout);
			if (getStringInput(path, 0, sizeof(path), NULL)) return;
			if (path) saveState(path);
		}
		else
		{
			nfdchar_t* out_path;
			nfdfilteritem_t filter_items[1] = { { "Binary data", "dat" } };
			nfdresult_t result = NFD_SaveDialog(&out_path, filter_items, 1, NULL, "save");
			if (result == NFD_OKAY)
			{
				saveState(out_path);
				NFD_FreePath(out_path);
			}
		}
#endif
		break;
	}
}

/**
 * @brief Computes the quick health based on each member's health
 * @return the string description
 */
static const char* getQuickHealth(void)
{
	// TODO
	return "good";
}

static void advanceDate()
{
	if (++state.day > MONTH_LENGTHS[state.month])
	{
		state.day = 1;
		state.month++;
	}
}

static void formatDate(char* buffer)
{
	byte len = (byte)strlen(MONTHS[state.month]);
	memcpy(buffer, MONTHS[state.month], len);
	buffer[len] = ' ';
	sprintf(buffer + len + 1, "%d", state.day);
	strcat(buffer, ", 1868");
}

static declare_choice_callback(map)
{
	putsn(ANSI_SB_ALT);
	showMap();
	putsn(ANSI_SB_MAIN);
	showMain();
}

static declare_choice_callback(trail)
{
	struct WrapLine* lines = NULL;
	cvector_init(lines, 0, NULL);

	addStaticLine(lines, "Date:", WRAPLINEKIND_RTL);
	addStaticLine(lines, "Weather:", WRAPLINEKIND_RTL);

	clearStdout();
	putBlockWL(lines, 1, SCREEN_HEIGHT - (byte)cvector_size(lines), SCREEN_WIDTH / 2);

	lines = NULL;
	cvector_init(lines, 0, NULL);
	char date[16];
	formatDate(date);

	lines = addLine(lines, date, WRAPLINEKIND_LTR);
	lines = addLine(lines, WEATHERS[state.weather], WRAPLINEKIND_LTR);

	putBlockWL(lines, SCREEN_WIDTH / 2 + 2, SCREEN_HEIGHT - (byte)cvector_size(lines), 0);
	fflush(stdout);
}

const struct ChoiceDialogChoice SETTLEMENT_CHOICES[] = {
	{.name = "Continue on trail", .callback = choice_callback(trail)},
	{.name = "Check supplies"},
	{.name = "Look at map", .callback = choice_callback(map)}
};

/**
 * @return the new pointer to <lines>
 */
static struct WrapLine* addQuickInfo(struct WrapLine* lines)
{
	lines = addBar(lines);

	// Weather
	char buffer[32] = "Weather: ";
	strcat(buffer, WEATHERS[state.weather]);
	lines = addLine(lines, buffer, WRAPLINEKIND_LTR);

	// Health
	strcpy(buffer, "Health: ");
	strcat(buffer, getQuickHealth());
	lines = addLine(lines, buffer, WRAPLINEKIND_LTR);

	// Pace
	strcpy(buffer, "Pace: ");
	strcat(buffer, PACES[state.pace]);
	lines = addLine(lines, buffer, WRAPLINEKIND_LTR);

	// Ration
	strcpy(buffer, "Rations: ");
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
	formatDate(date);

	lines = addLine(lines, date, WRAPLINEKIND_CENTER);
	lines = addNewline(lines);

	lines = addQuickInfo(lines);

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
	if (errno) return;
	state.stage = STATE_STAGE_START;
	autoSave();
	showMain();
}

static declare_choice_callback_g(month)
{
	state.month = index;



	char text[256];

	sprintf(text, "Before leaving Independence you should buy equipment and supplies. You have $%.2f in cash, but you don't have to spend it all now.", state.money);
	showInfoDialog(NULL, text);
	if (errno) return;
	showInfoDialog(NULL, "You can buy whatever you need at Matt's General Store.");
	if (errno) return;
#define matt_greeting "Hello, I'm Matt. So you're going to Oregon! I can fix you up with what you need:\n\n"
	showInfoDialog("Meet Matt", matt_greeting TAB"- a team of oxen to pull\n"TAB"  your wagon\n\n"TAB"- clothing for both\n"TAB"  summer and winter");
	if (errno) return;
	showInfoDialog("Meet Matt", matt_greeting TAB"- plenty of food for the\n"TAB"  trip\n\n"TAB"- ammunition for your\n"TAB"  rifles\n\n"TAB"- spare parts for your\n"TAB"  wagon");
	if (errno) return;
#undef matt_greeting
	showStore();
}

static declare_choice_callback(month_advice)
{
	showInfoDialog("Month Info", "You attend a public meeting held for \"folks with the California - Oregon fever.\" You're told:\n\nIf you leave too early, there won't be any grass for your oxen to eat. If you leave too late, you may not get to Oregon before winter comes. If you leave at just the right time, there will be green grass and the weather will still be cool.");
	if (errno) return;
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
	if (errno) return;
	showRole();
}

static enum QKeyCallbackReturn nameInputCallback(int key, va_list args)
{
	if (KEY_IS_TERMINATING(key)) return QKEY_CALLBACK_RETURN_END;

	if (key == '\b' || key == '\r' || key == DEL_CHAR || key == ' ' || key == '\'' || (KEY_IS_NORMAL(key) && isalpha(key))) return QKEY_CALLBACK_RETURN_NORMAL;
	return QKEY_CALLBACK_RETURN_IGNORE;
}

static declare_choice_callback_g(role)
{
	state.role = index;
	// money per role
	state.money = (float[]){ 1600.f, 800.f, 400.f } [index] ;

	clearStdout();
	drawBox(&("What is the first name of the wagon leader?\n" ANSI_CURSOR_SAVE)[0], DIALOG_WIDTH, BORDER_DOUBLE, &(struct _BoxOptions){
		.height = 8,
			.color = ANSI_COLOR_YELLOW
	});
	putsn(ANSI_CURSOR_RESTORE ANSI_CURSOR_SHOW);
	fflush(stdout);

	if (getStringInput(&state.wagon_leader->name[0], 1, NAME_SIZE, &nameInputCallback)) return;

	// pick 4 random names without repeats
	char taken_names[WAGON_MEMBER_COUNT] = { 0 };
	for (byte i = 0; i < WAGON_MEMBER_COUNT; i++)
	{
		byte rand_i;
		do
		{
			rand_i = rand() % _countof(WAGON_MEMBER_NAMES);
		} while (taken_names[rand_i]);
		taken_names[rand_i] = 1;
		strcpy(state.wagon_members[i].name, WAGON_MEMBER_NAMES[rand_i]);
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

static declare_choice_callback(main_load)
{
	char path[FILENAME_MAX];
#ifdef _DEBUG
	strcpy(path, DEBUG_SAVE_PATH);
#else
	if (IS_TTY)
	{
		showPromptDialog("Enter path to previous save file:", path, sizeof(path));
	}
	else
	{
		nfdchar_t* out_path;
		nfdfilteritem_t filter_items[1] = { { "Binary data", "dat" } };
		nfdresult_t result = NFD_OpenDialog(&out_path, filter_items, 1, NULL);
		if (result == NFD_OKAY)
		{
			strcpy(path, out_path);
			NFD_FreePath(out_path);
		}
		else if (result == NFD_CANCEL)
		{
			showMainMenu();
			return;
		}
		else
		{
			goto error;
		}
	}
#endif
	if (!loadState(path))
	{
	error:
		showErrorDialog("Error loading save");
		showMainMenu();
	}
}

struct Setting main_settings[] = {
	{.name = "Skip tutorials", .p = (void**)&settings.no_tutorials, .type = SETTING_TYPE_BOOLEAN},
	{.name = "Auto save", .p = (void**)&settings.auto_save, .type = SETTING_TYPE_BOOLEAN},
	{.name = "Auto save path", .p = (void**)settings.auto_save_path, .type = SETTING_TYPE_PATH},
	{.name = "Auto screen size", .p = (void**)&settings.auto_screen_size, .type = SETTING_TYPE_BOOLEAN, .callback = &updateScreenSize},
	{.name = "Screen width", .p = (void**)&settings.screen_width, .type = SETTING_TYPE_NUMBER, .callback = &updateScreenSize, .min = 30},
	{.name = "Screen height", .p = (void**)&settings.screen_height, .type = SETTING_TYPE_NUMBER, .callback = &updateScreenSize, .min = 10}
};

static declare_choice_callback(settings)
{
	showSettings(main_settings, _countof(main_settings));
	showMainMenu();
}

static declare_choice_callback(main_learn)
{
	showInfoDialog("Oregon Trail Info", "idk lol\n\n\n");
	if (errno) return;
	showMainMenu();
}

static declare_choice_callback(main_exit)
{
	errno = ENOENT;
}

const struct ChoiceDialogChoice main_choices[] = {
	{ANSI_COLOR_CYAN"Start traveling trail"ANSI_COLOR_RESET,.callback = choice_callback(main_start)},
	{"Continue traveling trail",.callback = choice_callback(main_load)},
	{"Learn about the trail", .callback = choice_callback(main_learn)},
	{"Change settings", .callback = choice_callback(settings)},
	{"Exit",.callback = choice_callback(main_exit)}
};
void showMainMenu(void)
{
	showChoiceDialog("You may:", main_choices, _countof(main_choices), &(struct _DialogOptions){
		.title = "Welcome to Oregon Trail"
	});
}

const struct ChoiceDialogChoice tutorial_choices[] = {
	{"Press one then enter"},
	{"Or up or down arrows, then enter"},
	{"Page up and down also work while selecting"}
};

#include "blackjack/game.h"
int main(void)
{
	setup();
	state.money = 30.f;
	enum BlackjackGameResult result = playBlackjack();
	getchar();

	loadSettings();

	if (!settings.no_tutorials)
	{
		showChoiceDialog("Here is a choice dialog:\n\nPress escape once to exit selection mode, and twice to exit a game.", tutorial_choices, _countof(tutorial_choices), NULL);
		settings.no_tutorials = 1;
		saveSettings();
	}

	while (1)
	{
		showMainMenu();
		//showStore();
		//showMap();
		//showMain();

		if (errno) break;
		if (state.stage)
		{
			showSavePrompt();
			state.stage = STATE_STAGE_NONE;
		}
	}

	setdown();
}

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "cvector.h"
#ifndef TOT_TTY
#include "nfd.h"
#endif
#ifndef TOT_MUTE
#include "music.h"
#endif

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
#include "blackjack/game.h"

void showMainMenu(void);
void showMonth(void);
void showRole(void);
void showSavePrompt(void);

void showSavePrompt(void)
{
	bool result = showConfirmationDialog("Would you like to save? " CONTROL_CHAR_STR);
	if (HALT)
		return;
	if (result)
	{
#ifndef _DEBUG
		saveState(DEBUG_SAVE_PATH);
#else
		if (IS_TTY)
		{
			char path[FILENAME_MAX];
			clearStdout();
			puts("Enter a path:");
			fflush(stdout);
			if (getStringInput(path, 0, sizeof(path), NULL))
				return;
			if (path)
				saveState(path);
		}
#ifndef TOT_TTY
		else
		{
			nfdchar_t* out_path;
			nfdresult_t result = NFD_SaveDialog(&out_path, &SAVE_FILE_NFD_FILTER_ITEM, 1, NULL, "save");
			if (result == NFD_OKAY)
			{
				saveState(out_path);
				NFD_FreePath(out_path);
			}
		}
#endif
#endif
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
	showMap();
	if (HALT)
		return;
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

static declare_choice_callback(supplies)
{
	// TODO
}

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
	const static struct ChoiceDialogChoice choices[] = {
	{.name = "Continue on trail", .callback = choice_callback(trail)},
	{.name = "Check supplies", .callback = choice_callback(supplies)},
	{.name = "Look at map", .callback = choice_callback(map)} };
	struct WrapLine* lines = NULL;
	cvector_init(lines, 0, NULL);

	char date[16];
	formatDate(date);

	lines = addLine(lines, date, WRAPLINEKIND_CENTER);
	lines = addNewline(lines);

	lines = addQuickInfo(lines);

	showChoiceDialogWL(lines, &choices[0], countof(choices), &(struct _DialogOptions){.title = state.location, .noPaddingY = 1});
}

static declare_choice_callback(blackjack)
{
	while (1)
	{
		enum BlackjackResult r = playBlackjack(10.5f);
		tot_sleep(1000);
		clearStdout();
		puts(getBlackjackResult(r));
		fflush(stdout);
		tot_sleep(1000);
		if (r == BLACKJACK_RESULT_QUIT) break;
	}
}

static declare_choice_callback_g(month)
{
	const static struct StoryPage story[] = {
	{.title = "Dim morning", .text = "You wander into Independence as the weak sun breaks the horizon, horse in toe. The journey has here was tiresome, but in reality, this was just the beginning. Ahead, a stout man leans against a post beside the road."},
	{.title = "Meeting", .text = "\"Ho!\" exclaims the man, \"What brings you to Independence?\""} };
	state.month = index;

	showStoryDialog(story, countof(story));
	if (HALT)
		return;
	struct ChoiceDialogChoice choices[] = {
		{.name = (char[32]){0}},
		{.name = "\"Mind your own hoppin' business!\""},
		{.name = "\"I challenge you to a game or two of blackjack!\"", .callback = choice_callback(blackjack)} };
	sprintf(choices[0].name, "\"My name is %s.\"", state.wagon_leader->name);
	putsn(ANSI_CURSOR_SHOW);
	showChoiceDialog("What are you doing?", choices, countof(choices), NULL);
	if (HALT)
		return;

	playBlackjack(20.f);

	char text[256];

	sprintf(text, "Before leaving Independence you should buy equipment and supplies. You have $%.2f in cash, but you don't have to spend it all now.", state.money);
	showInfoDialog(NULL, text);
	if (HALT)
		return;
	showInfoDialog(NULL, "You can buy whatever you need at Matt's General Store.");
	if (HALT)
		return;
	showLongInfoDialog("Meet Matt", "Hello, I'm Matt. So you're going to Oregon! I can fix you up with what you need:\n\n" TAB "- plenty of food for the\n" TAB "  trip\n\n" TAB "- ammunition for your\n" TAB "  rifles\n\n" TAB "- spare parts for your\n" TAB "  wagon\n" TAB "- a team of oxen to pull\n" TAB "  your wagon\n\n" TAB "- clothing for both\n" TAB "  summer and winter", COLOR_YELLOW);
	if (HALT)
		return;
#undef matt_greeting

	// set location
	strcpy(state.location, "Independence, Missouri");

	showStore();
	if (HALT)
		return;

	showInfoDialog("Parting with Matt", "Well then, you're ready to go. Good luck! You have a long and difficult journey ahead of you.");
	if (HALT)
		return;
	state.stage = STATE_STAGE_START;
	setActivity("On the trail");
	autoSave();
	showMain();
}

static declare_choice_callback(month_advice)
{
	showInfoDialog("Month Info", "You attend a public meeting held for \"folks with the California - Oregon fever.\" You're told:\n\nIf you leave too early, there won't be any grass for your oxen to eat. If you leave too late, you may not get to Oregon before winter comes. If you leave at just the right time, there will be green grass and the weather will still be cool.");
	if (HALT)
		return;
	showMonth();
}

void showMonth(void)
{
	const static char text[] = "It is 1848. Your jumping off place for Oregon is Independence, Missouri. You must decide which month to leave Independence.";
	const static struct ChoiceDialogChoice choices[] = {
	{"March"},
	{"April"},
	{"May"},
	{"June"},
	{"July"},
	{"Ask for advice", .callback = choice_callback(month_advice)} };

	showChoiceDialog(text, choices, countof(choices), &(struct _DialogOptions){.callback = choice_callback(month)});
}

static declare_choice_callback(role_learn)
{
	showLongInfoDialog("Role Info", "Traveling to Oregon isn't easy! But if you're a banker, you'll have more money for supplies and services than a carpenter or a farmer.\n\nHowever, the harder you have to try, the more points you deserve! Therefore, the farmer earns the greatest number of points and the banker earns the least.", COLOR_DEFAULT);
	if (HALT)
		return;
	showRole();
}

static enum QKeyCallbackReturn nameInputCallback(int key, va_list args)
{
	if (key == '\b' || key == ETR_CHAR || key == DEL_CHAR || key == ' ' || key == '\'' || (KEY_IS_NORMAL(key) && isalpha(key)))
		return QKEY_CALLBACK_RETURN_NORMAL;
	return QKEY_CALLBACK_RETURN_IGNORE;
}

static declare_choice_callback_g(role)
{
	state.role = index;
	// money per role
	state.money = (float[]){ 1600.f, 800.f, 400.f } [index] ;

	clearStdout();
	drawBox(&("What is the first name of the wagon leader?\n" ANSI_CURSOR_SAVE)[0], DIALOG_WIDTH, BORDER_DOUBLE, &(struct _BoxOptions){.height = 8, .color = COLOR_YELLOW});
	putsn(ANSI_CURSOR_RESTORE ANSI_CURSOR_SHOW);
	fflush(stdout);

	if (getStringInput(&state.wagon_leader->name[0], 1, NAME_SIZE, &nameInputCallback))
		return;

	// pick 4 random names without repeats
	char taken_names[WAGON_MEMBER_COUNT] = { 0 };
	for (byte i = 0; i < WAGON_MEMBER_COUNT; i++)
	{
		byte rand_i;
		do
		{
			rand_i = rand() % countof(WAGON_MEMBER_NAMES);
		} while (taken_names[rand_i]);
		taken_names[rand_i] = 1;
		strcpy(state.wagon_members[i].name, WAGON_MEMBER_NAMES[rand_i]);
	}

	showMonth();
}

void showRole(void)
{
	const static struct ChoiceDialogChoice choices[] = {
	{"Be a banker from Boston"},
	{"Be a carpenter from Ohio"},
	{"Be a farmer from Illinois"},
	{"Find out the differences between these choices", .callback = choice_callback(role_learn)} };

	setActivity("Getting started");
	showChoiceDialog("Many kinds of people made the trip to Oregon.\n\nYou may:", choices, countof(choices), &(struct _DialogOptions){.callback = choice_callback(role)});
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
#ifndef TOT_TTY
	else
	{
		nfdchar_t* out_path;
		nfdresult_t result = NFD_OpenDialog(&out_path, &SAVE_FILE_NFD_FILTER_ITEM, 1, NULL);
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
#endif
	if (loadState(path))
	{
#ifndef _DEBUG
		error :
#endif
		showErrorDialog("Error loading save");
		if (HALT) return;
		showMainMenu();
	}
}

static void discord_toggle(void)
{
	if (settings.discord_rp)
	{
		discord_setdown();
	}
	else
	{
		discord_setup();
	}
}

static declare_choice_callback(settings)
{
	const static struct Setting main_settings[] = {
#ifndef TOT_MUTE
	   {.name = "Volume", .p = (void**)&settings.volume, .type = SETTING_TYPE_FRACTIONAL, .max = 10},
#endif
	   {.name = "Skip tutorials", .p = (void**)&settings.no_tutorials, .type = SETTING_TYPE_BOOLEAN},
	   {.name = "Auto save", .p = (void**)&settings.auto_save, .type = SETTING_TYPE_BOOLEAN},
	   {.name = "Auto save path", .p = (void**)settings.auto_save_path, .type = SETTING_TYPE_PATH},
	   {.name = "Auto screen size", .p = (void**)&settings.auto_screen_size, .type = SETTING_TYPE_BOOLEAN, .callback = &updateAutoScreenSize},
	   {.name = "Screen width", .p = (void**)&settings.screen_width, .type = SETTING_TYPE_NUMBER, .callback = &updateScreenSize, .min = 30},
	   {.name = "Screen height", .p = (void**)&settings.screen_height, .type = SETTING_TYPE_NUMBER, .callback = &updateScreenSize, .min = 10},
   #ifdef TOT_DISCORD
	   {.name = "Enable Discord rich presence", .p = (void**)&settings.discord_rp, .type = SETTING_TYPE_BOOLEAN, .callback = &discord_toggle}
   #endif
	};

	showSettings(main_settings, countof(main_settings));
	if (HALT == HALT_QUIT)
		return;
	showMainMenu();
}

static declare_choice_callback(main_learn)
{
	setActivity("Learning about the trail");
	showInfoDialog("Oregon Trail Info", "A TUI game heavily based on MECC's 1990 The Oregon Trail\n\n\n");
	if (HALT == HALT_QUIT)
		return;
	showMainMenu();
}

static declare_choice_callback(main_exit)
{
	HALT = HALT_QUIT;
}

void showMainMenu(void)
{
	const static struct ChoiceDialogChoice choices[] = {
	{ANSI_COLOR_CYAN "Start traveling trail" ANSI_COLOR_RESET, .callback = choice_callback(main_start)},
	{"Continue traveling trail", .callback = choice_callback(main_load)},
	{"Learn about the trail", .callback = choice_callback(main_learn)},
	{"Change settings", .callback = choice_callback(settings)},
	{"Exit", .callback = choice_callback(main_exit)} };

	setActivity("Pondering the main menu");
	showChoiceDialog("You may:", choices, countof(choices), &(struct _DialogOptions){.title = "The Oregon Trail"});
}


int main(void)
{
	if (setup())
		goto error;
	// TODO
	const char r = loadSettings();
	if (r > 1)
	{
		puts_warnf("Settings version %c unknown. Ignoring, and will write over.", r);
	}

	if (post_setup())
		goto error;

	if (!settings.no_tutorials)
	{
		const struct ChoiceDialogChoice tutorial_choices[] = {
		{"Press one then enter"},
		{"Or up or down arrows, then enter"},
		{"Page up and down also work while selecting"} };

		state.stage = STATE_STAGE_TUTORIAL;
		HALT = HALT_DISALLOWED;
		showChoiceDialog("Here is a choice dialog:\n\nPress escape once to exit choice selection mode, and twice to exit a game.", tutorial_choices, countof(tutorial_choices), NULL);
		if (HALT == HALT_QUIT)
			goto error;
		settings.no_tutorials = 1;
		saveSettings();
		state.stage = STATE_STAGE_NONE;
		HALT = HALT_NONE;
	}

#ifndef TOT_MUTE
	// if (music_play("../resources/sample2.wav")) goto error;
#endif

	putsn(ANSI_CURSOR_SHOW);
	fflush(stdout);
	/*char buf[64];
	Coord offset = { 0 };
	getWrappedStringInput(buf, 10, offset, 0, sizeof(buf), NULL);*/

	while (1)
	{
		showMainMenu();

		if (HALT == HALT_QUIT)
			break;
		if (HALT == HALT_GAME)
		{
			HALT = HALT_NONE;
			if (state.stage)
			{
				state.stage = STATE_STAGE_NONE;
				showSavePrompt();
				if (HALT == HALT_QUIT)
					break;
			}
		}
	}

error:
	setdown();
}

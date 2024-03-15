#include <malloc.h>
#include <assert.h>
#include <string.h>
#ifndef TOT_TTY
#include "nfd.h"
#endif

#include "settings.h"
#include "input.h"
#include "utils.h"

static const char SETTING_TYPE_NAMES[][8] = {
	"number",
	"boolean",
	"string",
	"path"
};

const nfdfilteritem_t SAVE_FILE_NFD_FILTER_ITEM = { "Binary data", "dat" };

static void printFormattedSetting(struct Setting* setting)
{
	putsn(ANSI_BOLD);
	switch (setting->type)
	{
	case SETTING_TYPE_NUMBER:
		printf("%i", *(int*)setting->p);
		break;

	case SETTING_TYPE_BOOLEAN:
		putsn(*(bool*)setting->p ? ANSI_COLOR_GREEN"True" : ANSI_COLOR_RED"False");
		break;

	case SETTING_TYPE_STRING:
	case SETTING_TYPE_PATH:
		putsn(*(char*)setting->p ? (char*)setting->p : ANSI_COLOR_GRAY"Empty");
		break;
	}
	puts(ANSI_COLOR_RESET);
}

struct Setting* gp_settings;
struct ChoiceDialogChoice* gp_dialog_choices;
byte gp_dialog_choices_count;
bool do_exit;

struct _DialogOptions dialog_options = {
	.color = COLOR_GREEN,
		.title = "Settings",
		.callback = &settingCallback
};

static enum QKeyCallbackReturn settingInputCallback(int key, va_list args)
{
	if (key == ESC_CHAR)
	{
		do_exit = 1;
		escape_combo = 0;
		return QKEY_CALLBACK_RETURN_END;
	}
	return QKEY_CALLBACK_RETURN_NORMAL;
}

void settingCallback(const struct ChoiceDialogChoice* choice, const int index)
{
	clearStdout();
	puts("Current value:");
	printFormattedSetting(&gp_settings[index]);
	putsn("Enter a ");
	putsn(SETTING_TYPE_NAMES[gp_settings[index].type]);
	puts(":"ANSI_CURSOR_SHOW);
	fflush(stdout);
	switch (gp_settings[index].type)
	{
	case SETTING_TYPE_NUMBER:
		int num = getNumberInput(gp_settings[index].min, elvis(gp_settings[index].max, -1), 1, settingInputCallback);
		if (num < 0) goto skip;
		*(int*)gp_settings[index].p = num;
		break;

	case SETTING_TYPE_STRING:
		char buffer[32];
		if (getStringInput(buffer, 0, sizeof(buffer), settingInputCallback)) goto skip;
		memcpy(gp_settings[index].p, buffer, sizeof(buffer));
		break;

	case SETTING_TYPE_PATH:
	{
		if (IS_TTY)
		{
			char buffer[FILENAME_MAX];
			if (getStringInput(buffer, 0, sizeof(buffer), settingInputCallback)) goto skip;
			memcpy(gp_settings[index].p, buffer, sizeof(buffer));
		}
#ifndef TOT_TTY
		else
		{
			nfdchar_t* out_path;
			putsn(ANSI_CURSOR_HIDE);
			fflush(stdout);
			nfdresult_t result = NFD_SaveDialog(&out_path, &SAVE_FILE_NFD_FILTER_ITEM, 1, NULL, "save");
			putsn(ANSI_CURSOR_SHOW);
			if (result == NFD_OKAY)
			{
				strcpy((char*)gp_settings[index].p, out_path);
				NFD_FreePath(out_path);
			}
			else
			{
				goto skip;
			}
		}
#endif
	}
	break;

	case SETTING_TYPE_BOOLEAN:
		do_exit = 0;
		bool b = getBooleanInput(settingInputCallback);
		if (HALT) return;
		if (do_exit) break;
		*(bool*)gp_settings[index].p = b;
		break;
	}
	if (gp_settings[index].callback) gp_settings[index].callback();
	saveSettings();
skip:
	if (HALT) return;
	showChoiceDialog("Compiled "__DATE__, gp_dialog_choices, gp_dialog_choices_count, &dialog_options);
}

static void exitCallback(const struct ChoiceDialogChoice* choice) {}

void showSettings(struct Setting* settings, byte settings_count)
{
	struct ChoiceDialogChoice* choices = malloc(((unsigned short)settings_count + 1) * sizeof(struct ChoiceDialogChoice));
	assert(choices);
	for (byte i = 0; i < settings_count; i++)
	{
		choices[i].name = (char*)settings[i].name;
		choices[i].callback = NULL;
	}
	choices[settings_count].name = "Back";
	choices[settings_count].callback = &exitCallback;
	gp_settings = settings;
	gp_dialog_choices = choices;
	gp_dialog_choices_count = settings_count + 1;
	showChoiceDialog("Compiled "__DATE__, choices, gp_dialog_choices_count, &dialog_options);
	free(choices);
}

void autoSave(void)
{
	if (settings.auto_save && settings.auto_save_path) saveState(settings.auto_save_path);
}
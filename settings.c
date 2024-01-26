#include <malloc.h>
#include <assert.h>
#include <string.h>

#include "settings.h"
#include "input.h"
#include "utils.h"

const char SETTING_TYPE_NAMES[][8] = {
	"number",
	"boolean",
	"string",
	"path"
};

static void printFormattedSetting(struct Setting* setting)
{
	switch (setting->type)
	{
	case SETTING_TYPE_NUMBER:
		printf("%i\n", *(int*)setting->p);
		break;

	case SETTING_TYPE_BOOLEAN:
		puts(*(bool*)setting->p ? "True" : "False");
		break;

	case SETTING_TYPE_STRING:
	case SETTING_TYPE_PATH:
		puts(*(char*)setting->p ? (char*)setting->p : ANSI_COLOR_RED"Empty"ANSI_COLOR_RESET);
		break;
	}
}

struct Setting* gp_settings;
struct ChoiceDialogChoice* gp_dialog_choices;
byte gp_dialog_choices_count;
bool do_exit;

struct _DialogOptions dialog_options = {
	.color = ANSI_COLOR_GREEN,
		.title = "Settings",
		.callback = &settingCallback
};

static enum QKeyCallbackReturn settingInputCallback(unsigned key, enum QKeyType type, va_list args)
{
	if (type == QKEY_TYPE_NORMAL && key == ESCAPE_CHAR)
	{
		do_exit = 1;
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
		int num = getNumberInput(0, -1, 1, settingInputCallback);
		if (num >= 0) goto skip;
		*(int*)gp_settings[index].p = num;
		break;

	case SETTING_TYPE_STRING:
		char buffer[32];
		if (getStringInput(&buffer, 0, sizeof(buffer), settingInputCallback)) goto skip;
		memcpy(gp_settings[index].p, buffer, sizeof(buffer));
		break;

	case SETTING_TYPE_PATH:
	{
		// TODO: open file dialog for GUI
		char buffer[FILENAME_MAX];
		if (getStringInput(&buffer, 0, sizeof(buffer), settingInputCallback)) goto skip;
		memcpy(gp_settings[index].p, buffer, sizeof(buffer));
	}
	break;

	case SETTING_TYPE_BOOLEAN:
		do_exit = 0;
		bool b = getBooleanInput(settingInputCallback);
		if (do_exit) break;
		*(bool*)gp_settings[index].p = b;
		break;
	}
	saveSettings();
skip:
	showChoiceDialog("Compiled "__DATE__, gp_dialog_choices, gp_dialog_choices_count, &dialog_options);
}

static void exitCallback(const struct ChoiceDialogChoice* choice) {}

void showSettings(struct Setting* settings, byte settings_count)
{
	struct ChoiceDialogChoice* choices = malloc(((unsigned short)settings_count + 1) * sizeof(struct ChoiceDialogChoice));
	assert(choices);
	for (byte i = 0; i < settings_count; i++)
	{
		choices[i].name = settings[i].name;
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
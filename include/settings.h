#pragma once
#include "base.h"
#include "state.h"
#include "tui.h"

enum SettingType
{
	SETTING_TYPE_NUMBER,
	SETTING_TYPE_FRACTIONAL,
	SETTING_TYPE_BOOLEAN,
	SETTING_TYPE_STRING,
	SETTING_TYPE_PATH
};

struct Setting
{
	const char name[32];
	const char description[48];
	void** p;
	const enum SettingType type;
	void (*callback)(void);
	unsigned min, max;
};

extern const nfdfilteritem_t SAVE_FILE_NFD_FILTER_ITEM;

void showSettings(const struct Setting* const settings, byte settings_count);
void settingCallback(const struct ChoiceDialogChoice* choice, const int index);
void autoSave(void);
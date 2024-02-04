#pragma once
#include "base.h"
#include "state.h"
#include "tui.h"

enum SettingType
{
	SETTING_TYPE_NUMBER,
	SETTING_TYPE_BOOLEAN,
	SETTING_TYPE_STRING,
	SETTING_TYPE_PATH
};

struct Setting
{
	const char name[32];
	void** p;
	const enum SettingType type;
	void (*callback)(void);
	unsigned min, max;
};

void showSettings(struct Setting* settings, byte settings_count);
void settingCallback(const struct ChoiceDialogChoice* choice, const int index);
void autoSave(void);
#pragma once
#include "base.h"
#include "tui.h"

enum SettingType
{
  SETTING_TYPE_NUMBER,
  SETTING_TYPE_FRACTIONAL,
  SETTING_TYPE_BOOLEAN,
  SETTING_TYPE_FIXED_STRING,
  SETTING_TYPE_DYNAMIC_STRING,
  SETTING_TYPE_PATH
};

typedef union
{
  int32_t number;
  uint32_t fractional;
  bool boolean;
  char* string;
}* SettingValuePointer;

struct Setting
{
  const char name[32];
  const char description[48];

  union
  {
    int32_t* number;
    uint32_t* fractional;
    bool* boolean;
    char* fixed_string;
    char** dynamic_string;
  } p;
  const enum SettingType type;
  void (*callback)(void);
  unsigned min;
  unsigned max;

  union
  {
    bool live, dynamic;
  };
};

#ifndef TOT_TTY
extern const nfdfilteritem_t SAVE_FILE_NFD_FILTER_ITEM;
#endif

void showSettings(byte settings_count, const struct Setting settings[settings_count]);
void settingCallback(const struct ChoiceDialogChoice*, const int);

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#ifndef TOT_TTY
#include "nfd.h"
#endif

#include "input.h"
#include "settings.h"
#include "state.h"
#include "utils.h"

const char SETTING_TYPE_NAMES[][8] = { "number", "", "boolean", "string", "path" };

#ifndef TOT_TTY
const nfdfilteritem_t SAVE_FILE_NFD_FILTER_ITEM = { "Binary data", "dat" };
#endif

static void
printFractionalSetting(const struct Setting* setting, unsigned value)
{
#define TOT_ASCII
  putchar('[');
  unsigned i = 0;
  while (i++ < value)
#ifdef TOT_ASCII
    putchar('|');
#else
    putsn("\u2588");
#endif
  while (i++ <= setting->max - setting->min)
#ifdef TOT_ASCII
    putchar('.');
#else
    putsn("\u22C5");
#endif
  putchar(']');
}

static void
printFormattedSetting(const struct Setting* setting)
{
  putsn(ANSI_BOLD);
  switch (setting->type)
  {
  case SETTING_TYPE_NUMBER:       printf("%i", *setting->p.number); break;

  case SETTING_TYPE_FRACTIONAL:   printFractionalSetting(setting, *setting->p.fractional); break;

  case SETTING_TYPE_BOOLEAN:      putsn(*setting->p.boolean ? ANSI_COLOR_GREEN "True" : ANSI_COLOR_RED "False"); break;

  case SETTING_TYPE_FIXED_STRING: putsn(setting->p.fixed_string ? setting->p.fixed_string : ANSI_COLOR_GRAY "Empty"); break;

  case SETTING_TYPE_DYNAMIC_STRING:
  case SETTING_TYPE_PATH:
    putsn(setting->p.dynamic_string && *setting->p.dynamic_string ? *setting->p.dynamic_string : ANSI_COLOR_GRAY "Empty");
    break;
  }
  puts(ANSI_COLOR_RESET);
}

const struct Setting* gp_settings;
struct ChoiceDialogChoice* gp_dialog_choices;
byte gp_dialog_choices_count;
bool do_exit;

struct DialogOptions dialog_options = { .color = COLOR_GREEN, .title = "Settings", .callback = &settingCallback };

static enum QKeyCallbackReturn
settingInputCallback(int key, va_list args)
{
  if (key == ESC_CHAR)
  {
    do_exit = 1;
    escape_combo = 0;
    return QKEY_CALLBACK_RETURN_END;
  }
  return QKEY_CALLBACK_RETURN_NORMAL;
}

void
settingCallback(const struct ChoiceDialogChoice* choice, const int index)
{
  clearStdout();
  if (gp_settings[index].description[0])
  {
    putBlockWL(wrapText(gp_settings[index].description, SCREEN_WIDTH, NULL), 0, 0, SCREEN_WIDTH);
    putchar('\n');
  }
  putsn("Press escape to cancel.\n\nCurrent value: " ANSI_CURSOR_SAVE);
  printFormattedSetting(&gp_settings[index]);
  if (gp_settings[index].type != SETTING_TYPE_FRACTIONAL)
  {
    putsn("Enter a ");
    putsn(SETTING_TYPE_NAMES[gp_settings[index].type]);
    putsn(": " ANSI_CURSOR_SHOW);
  }
  else
  {
    putsn("Use right and left arrow keys." ANSI_CURSOR_HIDE);
  }
  fflush(stdout);
  switch (gp_settings[index].type)
  {
  case SETTING_TYPE_NUMBER:;
    int num = getNumberInput(gp_settings[index].min, elvis(gp_settings[index].max, -1), 1, settingInputCallback);
    if (num < 0) goto skip;
    *gp_settings[index].p.number = num;
    break;

  case SETTING_TYPE_FRACTIONAL:;
    uint32_t* value = gp_settings[index].p.fractional;
    unsigned inital_value = *value;
    putsn(ANSI_BOLD);
    while (1)
    {
      int key = getKeyInput();
      if (KEY_IS_TERMINATING(key)) return;
      switch (key)
      {
      case ESC_CHAR:
        do_exit = 1;
        escape_combo = 0;
        *value = inital_value;
        if (gp_settings[index].callback) gp_settings[index].callback();
        goto skip;

      case KEY_ARROW_LEFT:
        if (*value > gp_settings[index].min)
        {
          putsn(ANSI_CURSOR_RESTORE);
          printFractionalSetting(&gp_settings[index], --*value);
          fflush(stdout);
          if (gp_settings[index].callback) gp_settings[index].callback();
        }
        break;

      case KEY_ARROW_RIGHT:
        if (*value < gp_settings[index].max)
        {
          putsn(ANSI_CURSOR_RESTORE);
          printFractionalSetting(&gp_settings[index], ++*value);
          fflush(stdout);
          if (gp_settings[index].callback) gp_settings[index].callback();
        }
        break;

      case ETR_CHAR: goto exit_fractional;
      }
    }
  exit_fractional:
    break;

  case SETTING_TYPE_FIXED_STRING:
  {
    char buffer[gp_settings[index].max];
    if (getStringInput(buffer, 0, gp_settings[index].max, settingInputCallback)) goto skip;
    strcpy(gp_settings[index].p.fixed_string, buffer);
    break;
  }

  case SETTING_TYPE_DYNAMIC_STRING:
    // TODO
    break;

  case SETTING_TYPE_PATH:
  {
    if (IS_TTY)
    {
      // TODO: use dynamic string
      char buffer[FILENAME_MAX];
      if (getStringInput(buffer, 0, sizeof(buffer), settingInputCallback)) goto skip;
      memcpy(gp_settings[index].p.fixed_string, buffer, sizeof(buffer));
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
        strcpy(gp_settings[index].p.string, out_path);
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
    *gp_settings[index].p.boolean = b;
    break;
  }
  if (gp_settings[index].callback) gp_settings[index].callback();
  saveSettings();
skip:
  if (HALT) return;
  showChoiceDialogWL(NULL, gp_dialog_choices_count, gp_dialog_choices, &dialog_options);
}

static void
exitCallback(const struct ChoiceDialogChoice* choice)
{
  // custom handler so it doesn't use the option type handler
  // purposefully empty!
}

void
showSettings(byte settings_count, const struct Setting settings[settings_count])
{
  struct ChoiceDialogChoice* choices = malloc(((size_t)settings_count + 1) * sizeof(struct ChoiceDialogChoice));
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

  showChoiceDialogWL(NULL, gp_dialog_choices_count, gp_dialog_choices, &dialog_options);
  free(choices);
}

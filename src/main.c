#include <stdio.h>

#include "base.h"
#include "screens.h"
#include "state.h"
#include "tui.h"

#include "setup.c"
#ifndef TOT_MUTE
#include "tracks.c"
#endif

void showSavePrompt(void);

void
screen_savePrompt(void)
{
  bool result = showConfirmationDialog("Would you like to save? " CAPTURE_STRING);
  if (HALT) return;
  if (result)
  {
#ifdef DEBUG
    saveState(DEBUG_SAVE_PATH);
#else
    if (IS_TTY)
    {
      char path[FILENAME_MAX];
      clearStdout();
      puts("Enter a path:");
      fflush(stdout);
      if (getStringInput(path, 0, sizeof(path), NULL)) return;
      if (*path) saveState(path);
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

static declare_choice_callback(main_start) { screen_role(); }

static declare_choice_callback(main_load)
{
  char path[FILENAME_MAX];
#if defined(DEBUG) && 0
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
      return;
    }
    else
    {
      goto error;
    }
  }
#endif
#endif
  const char* err = loadState(path);
  if (err)
  {
#ifndef DEBUG
  error:
#endif
    showErrorDialog("Error loading save", err);
  }
}

static declare_choice_callback(main_settings) { screen_mainSettings(); }

static declare_choice_callback(main_learn) { screen_learn(); }

static declare_choice_callback(main_exit) { HALT = HALT_QUIT; }

void
screen_mainMenu(void)
{
  static const struct ChoiceDialogChoice choices[] = {
    { ANSI_COLOR_CYAN "Start traveling trail" ANSI_COLOR_RESET, .callback = choice_callback(main_start)    },
    { "Continue traveling trail",                               .callback = choice_callback(main_load)     },
    { "Learn about the trail",                                  .callback = choice_callback(main_learn)    },
    { "Change settings",                                        .callback = choice_callback(main_settings) },
    { "Exit",                                                   .callback = choice_callback(main_exit)     }
  };

  setActivity("Pondering the main menu");
  showChoiceDialog("You may:", countof(choices), choices, &(struct DialogOptions){ .title = "The Oregon Trail" });
}

int
main(int argc, char** argv)
{
  bool prefer_tty = false;
  for (int i = 0; i < argc; i++)
  {
    if (strcmp(argv[i], "config") == 0)
    {
      loadSettings();
      puts(getSettingsPath());
      return 0;
    }
    if (strcmp(argv[i], "tty") == 0) prefer_tty = true;
  }

  if (setup(prefer_tty)) goto error;
  int error = loadSettings();
  if (error)
  {
    puts_warnf("Failed to load settings: %i.", error);
  }

  if (post_setup()) goto error;

  if (!settings.no_tutorials)
  {
    const struct ChoiceDialogChoice tutorial_choices[] = {
      { "Press one then enter" },
      { "Or up or down arrows, then enter" },
      { "Page up and down also work while selecting" },
    };

    state.stage = STATE_STAGE_TUTORIAL;
    HALT = HALT_DISALLOWED;
    showChoiceDialog(
        "Here is a choice dialog:\n\nPress escape once to exit choice selection mode, and twice to exit a game.",
        countof(tutorial_choices), tutorial_choices, NULL
    );
    if (HALT == HALT_QUIT) goto error;
    settings.no_tutorials = 1;
    saveSettings();
    state.stage = STATE_STAGE_NONE;
    HALT = HALT_NONE;
  }

#ifndef TOT_MUTE
  music_play(&track0, true);
#endif

  putsn(ANSI_CURSOR_SHOW);

  while (1)
  {
    screen_mainMenu();

    if (HALT == HALT_QUIT) break;
    if (HALT == HALT_GAME)
    {
      HALT = HALT_NONE;
      if (state.stage)
      {
        state.stage = STATE_STAGE_NONE;
        screen_savePrompt();
        if (HALT == HALT_QUIT) break;
      }
    }
  }

error:
  setdown();
}

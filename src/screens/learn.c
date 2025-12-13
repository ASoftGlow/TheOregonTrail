#include "licenses.h"
#include "screens.h"
#include "state.h"
#include "tui.h"

static bool back = 0;

static declare_choice_callback(back) { back = 1; }

static declare_choice_callback_g(license) { showLongInfoDialog(LICENSES[index].name, LICENSES[index].terms, COLOR_DEFAULT); }

static declare_choice_callback(licenses)
{
  struct ChoiceDialogChoice choices[LICENSES_COUNT + 1];
  for (unsigned i = 0; i < LICENSES_COUNT; i++)
  {
    choices[i].name = LICENSES[i].name;
    choices[i].callback = NULL;
  }
  choices[LICENSES_COUNT].name = "Back";
  choices[LICENSES_COUNT].callback = choice_callback(back);

  do
    showChoiceDialogWL(
        NULL, countof(choices), choices,
        &(struct DialogOptions){
            .title = "Dependency Licenses",
            .callback = choice_callback(license),
        }
    );
  while (!back && !HALT);
  back = 0;
}

void
screen_learn(void)
{
  static const struct ChoiceDialogChoice choices[] = {
    { "Licenses", .callback = choice_callback(licenses) },
    { "Back",     .callback = choice_callback(back)     },
  };
  setActivity("Learning about the trail");
  do
    showChoiceDialog(
        "A TUI game heavily based on MECC's 1990 The Oregon Trail\n\n\n", countof(choices), choices,
        &(struct DialogOptions){ .title = "Oregon Trail Info" }
    );
  while (!back && !HALT);
  back = 0;
}

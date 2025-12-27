#include "licenses.h"
#include "screens.h"
#include "state.h"
#include "tui.h"

static void
showLicenses(void)
{
  VLA(struct ChoiceDialogChoice, choices, LICENSES_COUNT + 1);
  for (unsigned i = 0; i < LICENSES_COUNT; i++)
  {
    choices[i].name = LICENSES[i].name;
  }
  choices[LICENSES_COUNT].name = "Back";

  while (1)
  {
    int choice = showChoiceDialogWL(
        NULL, LICENSES_COUNT + 1, choices,
        &(struct DialogOptions){
            .title = "Dependency Licenses",
        }
    );
    if (choice < 0 || (unsigned)choice == LICENSES_COUNT) break;
    showLongInfoDialog(LICENSES[choice].name, LICENSES[choice].terms, COLOR_DEFAULT);
  }
}

void
screen_learn(void)
{
  static const struct ChoiceDialogChoice choices[] = {
    { "Licenses" },
    { "Back" },
  };
  setActivity("Learning about the trail");

  while (1)
  {
    switch (showChoiceDialog(
        "A TUI game heavily based on MECC's 1990 The Oregon Trail\n\n\n", countof(choices), choices,
        &(struct DialogOptions){ .title = "Oregon Trail Info" }
    ))
    {
    case 0:  showLicenses(); break;
    case 1:
    case -1: return;
    }
  }
}

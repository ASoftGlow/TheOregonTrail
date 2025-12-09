#include "cvector/cvector.h"

#include "screens.h"
#include "state.h"
#include "static.h"
#include "tui.h"
#include "utils.h"

/**
 * @brief Computes the quick health based on each member's health
 * @return the string description
 */
static const char*
getQuickHealth(void)
{
  // TODO
  return "good";
}

static void
advanceDate()
{
  if (++state.day > MONTH_LENGTHS[state.month])
  {
    state.day = 1;
    state.month++;
  }
}

static void
formatDate(char* buffer)
{
  sprintf(buffer, "%s %d, 1868", MONTHS[state.month], state.day);
}

static declare_choice_callback(travel)
{
  struct WrapLine* lines = NULL;
  cvector_init(lines, 0, NULL);

  lines = addLine(lines, "Date:", WRAPLINEKIND_RTL);
  lines = addLine(lines, "Weather:", WRAPLINEKIND_RTL);

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
static struct WrapLine*
addQuickInfo(struct WrapLine* lines)
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

declare_choice_callback(map) { screen_map(); }

void
screen_trail(void)
{
  const static struct ChoiceDialogChoice choices[] = {
    { .name = "Continue on trail", .callback = choice_callback(travel)   },
    { .name = "Check supplies",    .callback = choice_callback(supplies) },
    { .name = "Look at map",       .callback = choice_callback(map)      }
  };
  struct WrapLine* lines = NULL;
  cvector_init(lines, 0, NULL);

  char date[16];
  formatDate(date);

  lines = addLine(lines, date, WRAPLINEKIND_CENTER);
  lines = addNewline(lines);

  lines = addQuickInfo(lines);
  lines = addNewline(lines);

  showChoiceDialogWL(lines, countof(choices), choices, &(struct DialogOptions){ .title = state.location, .noPaddingY = 1 });
}

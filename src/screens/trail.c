#include <string.h>

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
advanceDate(void)
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

static void
showTravel(void)
{
  FormattedLines lines = formatted_lines_create(2);

  lines = addLine(lines, "Date:", WRAPLINEKIND_RTL);
  lines = addLine(lines, "Weather:", WRAPLINEKIND_RTL);

  clearStdout();
  putBlockWL(lines, 1, SCREEN_HEIGHT - (byte)formatted_lines_size(lines), SCREEN_WIDTH / 2);

  formatted_lines_clear(lines);
  char date[16];
  formatDate(date);

  lines = addLine(lines, date, WRAPLINEKIND_LTR);
  lines = addLine(lines, WEATHERS[state.weather], WRAPLINEKIND_LTR);

  putBlockWL(lines, SCREEN_WIDTH / 2 + 2, SCREEN_HEIGHT - (byte)formatted_lines_size(lines), 0);
  fflush(stdout);
}

static void
showSupplies(void)
{
  // TODO
}

/**
 * @return the new pointer to <lines>
 */
static struct WrapLine*
addQuickInfo(struct WrapLine* lines)
{
  lines = addBar(lines, '-', COLOR_CYAN);

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

  lines = addBar(lines, '-', COLOR_CYAN);
  return lines;
}

void
screen_trail(void)
{
  static const struct ChoiceDialogChoice choices[] = {
    { .name = "Continue on trail" },
    { .name = "Check supplies" },
    { .name = "Look at map" },
  };

  while (1)
  {
    FormattedLines lines = formatted_lines_create(4);

    char date[16];
    formatDate(date);

    lines = addLine(lines, date, WRAPLINEKIND_CENTER);
    lines = addNewline(lines);

    lines = addQuickInfo(lines);
    lines = addNewline(lines);

    int choice = showChoiceDialogWL(
        lines, countof(choices), choices, &(struct DialogOptions){ .title = state.location, .noPaddingY = 1 }
    );
    switch (choice)
    {
    case -1: return;
    case 0:  showTravel(); break;
    case 1:  showSupplies(); break;
    case 2:  screen_map(); break;
    }
  }
}

#include <math.h>

#include "input.h"
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

static const char*
formatDate(void)
{
  static char date[16];
  sprintf(date, "%s %d, 1868", MONTHS[state.month], state.day);
  return date;
}

static void
showTravel(void)
{
  FormattedLines lines = formatted_lines_new();

  lines = addLine(lines, "Date:", WRAPLINEKIND_RTL);
  lines = addLine(lines, "Weather:", WRAPLINEKIND_RTL);

  clearStdout();
  putBlockWL(lines, 1, SCREEN_HEIGHT - (byte)formatted_lines_size(lines), SCREEN_WIDTH / 2);

  formatted_lines_clear(lines);

  lines = addLine(lines, formatDate(), WRAPLINEKIND_LTR);
  lines = addLine(lines, WEATHERS[state.weather], WRAPLINEKIND_LTR);

  putBlockWL(lines, SCREEN_WIDTH / 2 + 2, SCREEN_HEIGHT - (byte)formatted_lines_size(lines), 0);
  fflush(stdout);
}

static void
showSupplies(void)
{
  FormattedLines lines = formatted_lines_new();
  char buffer[32];

  lines = addLine(lines, "Your Supplies", WRAPLINEKIND_CENTER);
  lines = addNewline(lines);

  sprintf(buffer, "oxen             %4i", state.oxen);
  lines = addLine(lines, buffer, WRAPLINEKIND_CENTER);

  sprintf(buffer, "sets of clothing %4i", state.clothing_sets);
  lines = addLine(lines, buffer, WRAPLINEKIND_CENTER);

  sprintf(buffer, "bullets          %4i", state.bullets);
  lines = addLine(lines, buffer, WRAPLINEKIND_CENTER);

  sprintf(buffer, "wagon wheels     %4i", state.wagon_wheels);
  lines = addLine(lines, buffer, WRAPLINEKIND_CENTER);

  sprintf(buffer, "wagon axles      %4i", state.wagon_axles);
  lines = addLine(lines, buffer, WRAPLINEKIND_CENTER);

  sprintf(buffer, "wagon tongues    %4i", state.wagon_tongues);
  lines = addLine(lines, buffer, WRAPLINEKIND_CENTER);

  sprintf(buffer, "pounds of food   %4i", state.food);
  lines = addLine(lines, buffer, WRAPLINEKIND_CENTER);

  state.money = 10.5;
  // space at start to force cents off to the left
  sprintf(buffer, "   money left    %10.2f", state.money);
  // insert dollar prefix
  // - calculate num of digits
  //   - truncate
  //   - add decimal so input to log isn't zero
  // - calculate offset
  buffer[22 - (int)log10f((int)state.money + 0.5)] = '$';
  lines = addLine(lines, buffer, WRAPLINEKIND_CENTER);

  lines = addNewline(lines);
  lines = addLine(lines, "Press SPACE BAR to continue", WRAPLINEKIND_CENTER);

#define block_size 30
#if block_size > MIN_SCREEN_WIDTH
#error
#endif

  clearStdout();
  putsn(ANSI_CURSOR_HIDE);
  putBlockWL(lines, (SCREEN_WIDTH - block_size) / 2, 0, block_size);

  // art
  if (SCREEN_WIDTH - block_size >= 10)
  {
    // wagon wheel
    putBlock(
        ANSI_COLOR_BROWN "  __\n /\\/\\\n[=" ANSI_COLOR_GRAY "()" ANSI_COLOR_BROWN "=]\n \\" ANSI_UNDERLINE
                         "/\\" ANSI_NO_UNDERLINE "/",
        1, 1
    );
    //
    putBlock(
        ANSI_COLOR_RED "  ._.\n" ANSI_COLOR_BROWN "  \\ |\n  | |\n _] |\n" ANSI_UNDERLINE
                       "[~~~[" ANSI_NO_UNDERLINE ANSI_COLOR_GRAY "*" ANSI_COLOR_RESET,
        SCREEN_WIDTH - 6, 4
    );
  }

  fflush(stdout);
  waitForKey(' ');
  putsn(ANSI_CURSOR_SHOW);
}

CHECK_RETURN static FormattedLines
addQuickInfo(FormattedLines lines)
{
  const byte width = DIALOG_CONTENT_WIDTH;
  lines = justifyLineWL(lines, "Weather:", WEATHERS[state.weather], width);
  lines = justifyLineWL(lines, "Health:", getQuickHealth(), width);
  lines = justifyLineWL(lines, "Pace:", PACES[state.pace], width);
  lines = justifyLineWL(lines, "Rations:", RATIONS[state.ration], width);

  lines = addBar(lines, DIALOG_CONTENT_WIDTH, '~', COLOR_CYAN);
  return lines;
}

static void
changePace(void)
{
  static const struct ChoiceDialogChoice choices[] = {
    { "a steady pace" },
    { "a strenuous pace" },
    { "a grueling pace" },
    { "find out what these different paces mean" },
  };

#define STRING_TERM(x) ANSI_UNDERLINE x ANSI_NO_UNDERLINE
  const char* pace_info_text = STRING_TERM("steady"
  ) " - You travel about 8 hours a day, taking frequent rests. You take care not to get too tired.\n\n" STRING_TERM("strenuous"
  ) " - You travel about 12 hours a day, starting just after sunrise and stopping shortly before sunset. You stop to rest only "
    "when necessary. You finish each day feeling very tired.\n\n" STRING_TERM("grueling"
    ) " - You travel about 16 hours a day, starting begore sunrise and continuing until dark. You almost never stop to rest. "
      "You do not get enough sleep at night. You finish each day feeling absolutely exhausted, and your health suffers.";

  while (1)
  {
    int choice = showChoiceDialog(
        "The pace at which you travel can change. Your choices are:", countof(choices), choices,
        &(DialogOptions){ .title = "Change Pace" }
    );
    switch (choice)
    {
    case -1: return;
    case 3:  showLongInfoDialog("Pace Info", pace_info_text, COLOR_DEFAULT, false); break;
    default: state.pace = choice; return;
    }
  }
}

static void
changeRations(void)
{
  static const struct ChoiceDialogChoice choices[] = {
    { "filling - meals are large and generous." },
    { "meager - meals are small, but adequate." },
    { "bare bones - meals are very small; everyone stays hungry." },
  };

  int choice = showChoiceDialog(
      "The amount of food the people in your party eat each day can change. These amounts are:", countof(choices), choices,
      &(DialogOptions){ .title = "Change Food Rations" }
  );
  if (choice < 0) return;
  state.ration = choice;
}

void
drawPrompt(const char* text)
{
  putsn("\r" ANSI_CURSOR_FWD("5"));
  putsn(text);
}

static void
showRest(void)
{
  drawPrompt("How many days to rest? ");
  fflush(stdout);
  int days = getNumberInput(0, 7, false, NULL);
  if (days < 0) return;

  putsn(ANSI_CURSOR_HIDE);
  fflush(stdout);
  while (days--)
  {
    tot_sleep(1000);
    advanceDate();

    FormattedLines lines = formatted_lines_new();
    lines = addLine(lines, formatDate(), WRAPLINEKIND_CENTER);
    lines = addNewline(lines);
    lines = addQuickInfo(lines);

    putBlockWL(lines, 5, 1, DIALOG_CONTENT_WIDTH);
    fflush(stdout);
  }
  putsn(ANSI_CURSOR_SHOW);
}

void
screen_trail(void)
{
  static const struct ChoiceDialogChoice choices[] = {
    { .name = "Continue on trail" }, { .name = "Check supplies" },      { .name = "Look at map" },
    { .name = "Change pace" },       { .name = "Change food rations" }, { .name = "Stop to rest" },
    { .name = "Attempt to trade" },  { .name = "Talk to people" },      { .name = "Buy supplies" },
  };

  setActivity("On the trail");

  while (1)
  {
    FormattedLines lines = formatted_lines_new();

    lines = addLine(lines, formatDate(), WRAPLINEKIND_CENTER);
    lines = addNewline(lines);

    lines = addQuickInfo(lines);
    lines = addLine(lines, "You may:", WRAPLINEKIND_LTR);
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
    case 3:  changePace(); break;
    case 4:  changeRations(); break;
    case 5:  showRest(); break;
    }
  }
}

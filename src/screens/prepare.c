#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include "blackjack/game.h"
#include "input.h"
#include "screens.h"
#include "state.h"
#include "tui.h"
#include "utils.h"

static declare_choice_callback(blackjack)
{
  while (1)
  {
    enum BlackjackResult r = playBlackjack(10.5f);
    tot_sleep(1000);
    clearStdout();
    puts(getBlackjackResult(r));
    fflush(stdout);
    tot_sleep(1000);
    if (r == BLACKJACK_RESULT_QUIT) break;
  }
}

static declare_choice_callback_g(month)
{
  const struct StoryPage story[] = {
    { .title = "Dim morning",
     .text = "You wander into Independence as the weak sun breaks the horizon, horse in toe. The journey has here was "
              "tiresome, but "
              "in reality, this was just the beginning. Ahead, a stout man leans against a post beside the road." },
    { .title = "Meeting",     .text = "\"Ho!\" exclaims the man, \"What brings you to Independence?\""            }
  };
  state.month = index;
  showStoryDialog(countof(story), story);
  if (HALT) return;
  char name_intro[16 + NAME_SIZE];
  struct ChoiceDialogChoice choices[] = {
    { .name = name_intro },
    { .name = "\"Mind your own hoppin' business!\"" },
    { .name = "\"I challenge you to a game or two of blackjack!\"", .callback = choice_callback(blackjack) }
  };
  sprintf(name_intro, "\"My name is %s.\"", state.wagon_leader.name);
  putsn(ANSI_CURSOR_SHOW);

  showChoiceDialog("What are you doing?", countof(choices), choices, NULL);
  if (HALT) return;
  screen_generalStore();
}

static declare_choice_callback(month_advice)
{
  showInfoDialog(
      "Month Info",
      "You attend a public meeting held for \"folks with the California - Oregon fever.\" You're told:\n\nIf you leave too "
      "early, there won't be any grass for your oxen to eat. If you leave too late, you may not get to Oregon before "
      "winter "
      "comes. If you leave at just the right time, there will be green grass and the weather will still be cool."

  );
}

void
showMonth(void)
{
  const char text[] = "It is 1848. Your jumping off place for Oregon is Independence, Missouri. You must decide which "
                      "month to leave Independence.";
  const struct ChoiceDialogChoice choices[] = {
    { "March" },
    { "April" },
    { "May" },
    { "June" },
    { "July" },
    { "Ask for advice", .callback = choice_callback(month_advice) }
  };
  showChoiceDialog(text, countof(choices), choices, &(struct DialogOptions){ .callback = choice_callback(month) });
}

static declare_choice_callback(role_learn)
{
  showLongInfoDialog(
      "Role Info",
      "Traveling to Oregon isn't easy! But if you're a banker, you'll have more money for supplies and services than a "
      "carpenter or a farmer.\n\nHowever, the harder you have to try, the more points you deserve! Therefore, the farmer "
      "earns "
      "the greatest number of points and the banker earns the least.",
      COLOR_DEFAULT
  );
}

static enum QKeyCallbackReturn
nameInputCallback(int key, va_list args)
{
  (void)args;

  if (key == '\b' || key == ETR_CHAR || key == DEL_CHAR || key == ' ' || key == '\'' || (KEY_IS_NORMAL(key) && isalpha(key)))
    return QKEY_CALLBACK_RETURN_NORMAL;
  return QKEY_CALLBACK_RETURN_IGNORE;
}

static declare_choice_callback_g(role)
{
  state.role = index;
  // money per role
  state.money = (float[]){ 1600.f, 800.f, 400.f }[index];
  clearStdout();
  drawBox(
      &("What is the first name of the wagon leader?\n" ANSI_CURSOR_SAVE)[0], DIALOG_WIDTH, BORDER_DOUBLE,
      &(struct BoxOptions){ .height = 8, .color = COLOR_YELLOW }
  );
  putsn(ANSI_CURSOR_RESTORE ANSI_CURSOR_SHOW);
  fflush(stdout);
  if (getStringInput(state.wagon_leader.name, 1, NAME_SIZE, &nameInputCallback)) return;
  // pick 4 random names without repeats
  char taken_names[WAGON_MEMBER_COUNT] = { 0 };
  for (byte i = 0; i < WAGON_MEMBER_COUNT; i++)
  {
    byte rand_i;
    do {
      rand_i = rand() % countof(WAGON_MEMBER_NAMES);

    } while (taken_names[rand_i]);
    taken_names[rand_i] = 1;
    strcpy(state.wagon_members[i].name, WAGON_MEMBER_NAMES[rand_i]);
  }
  showMonth();
}

void
screen_role(void)
{
  const struct ChoiceDialogChoice choices[] = {
    { "Be a banker from Boston" },
    { "Be a carpenter from Ohio" },
    { "Be a farmer from Illinois" },
    { "Find out the differences between these choices", .callback = choice_callback(role_learn) }
  };

  setActivity("Getting started");
  showChoiceDialog(
      "Many kinds of people made the trip to Oregon.\n\nYou may:", countof(choices), choices,
      &(struct DialogOptions){ .callback = choice_callback(role) }
  );
}

#include <string.h>

#include "screens.h"
#include "state.h"
#include "store.h"
#include "tui.h"

static struct StoreCategory STORE_MATT_CATEGORIES[] = {
	{
		.name = "Oxen",
		.desciption = "There are 2 oxen in a yoke; I recommend at least 3 yoke. I charge $40 a yoke.",
		.image = "o-uu,",
		.items = (struct StoreItem[]) {
			{.name = "yokes", .price = 40.f, .min = 1, .max = 9}
		},
		.items_count = 1,
	},
	{
		.name = "Food",
		.desciption = "I recommend you take at least 200 pounds of food for each person in your family. I see that you have 5 people in all. You'll need flour, sugar, bacon, and coffee. My price is 20 cents a pound.",
		.image = "()O=-+\n    _\n   \\ \\",
		.items = (struct StoreItem[]) {
			{.name = "pounds of bacon", .price = .2f, .max = 1000},
			{.name = "pounds of flour", .price = .2f, .max = 1000}
		},
		.items_count = 2,
	},
	{
		.name = "Clothing",
		.desciption = "You'll need warm clothing in the mountains. I recommend taking at least 2 sets of clothes per person, $10.00 each.",
		.image = "__\n||",
		.items = (struct StoreItem[]) {
			{.name = "sets of clothes", .price = 10.f, .max = 99}
		},
		.items_count = 1,
	},
	{
		.name = "Ammunition",
		.desciption = "I sell ammunition in boxes of 20 bullets. Each box costs $2.00.",
		.image = "gun",
		.items = (struct StoreItem[]) {
			{.name = "boxes", .price = 2.f, .max = 99}
		},
		.items_count = 1,
	},
	{
		.name = "Spare parts",
		.desciption = "It's a good idea to have a few spare parts for your wagon. Here are the prices:\n\n    axle - $10\n   wheel - $10\n  tongue - $10",
		.image = "O _",
		.items = (struct StoreItem[]) {
			{.name = "axles", .price = 10.f, .max = 3},
			{.name = "wheels", .price = 10.f, .max = 3},
			{.name = "tongues", .price = 10.f, .max = 3},
		},
		.items_count = 3,
	}
};

static bool
store_matt_leave(const struct Store* store)
{
  // oxen
  if (store->categories[0].items[0].amount == 0)
  {
    showStoreAlert("Don't forget, you'll need oxen to pull your wagon.\n\n\n");
    return 1;
  }

  state.oxen = store->categories[0].items[0].amount * 2;
  state.food = store->categories[1].items[0].amount + store->categories[1].items[1].amount;
  state.clothing_sets = store->categories[2].items[0].amount;
  state.bullets = store->categories[3].items[0].amount * 20;
  state.wagon_axles = store->categories[4].items[0].amount;
  state.wagon_wheels = store->categories[4].items[1].amount;
  state.wagon_torques = store->categories[4].items[2].amount;
  return 0;
}

void
screen_generalStore(void)
{
  char text[256];
  sprintf(
      text,
      "Before leaving Independence you should buy equipment and supplies. You have $%.2f in cash, but you don't have to spend "
      "it all now.",
      state.money
  );
  showInfoDialog(NULL, text);
  if (HALT) return;
  showInfoDialog(NULL, "You can buy whatever you need at Matt's General Store.");
  if (HALT) return;
  showLongInfoDialog(
      "Meet Matt",
      "Hello, I'm Matt. So you're going to Oregon! I can fix you up with what you need:\n\n" //
      TAB "- plenty of food for the\n"                                                       //
      TAB "  trip\n\n"                                                                       //
      TAB "- ammunition for your\n"                                                          //
      TAB "  rifles\n\n"                                                                     //
      TAB "- spare parts for your\n"                                                         //
      TAB "  wagon\n"                                                                        //
      TAB "- a team of oxen to pull\n"                                                       //
      TAB "  your wagon\n\n"                                                                 //
      TAB "- clothing for both\n"                                                            //
      TAB "  summer and winter",
      COLOR_YELLOW
  );
  if (HALT) return;
#undef matt_greeting
  // set location
  strcpy(state.location, "Independence, Missouri");
  showStore(&(struct Store){
      .name = "Matt's General Store",
      .color = COLOR_CYAN,
      .color_category = COLOR_BLUE,
      .categories = STORE_MATT_CATEGORIES,
      .categories_count = countof(STORE_MATT_CATEGORIES),
      .callback_leave = &store_matt_leave,
  });
  if (HALT) return;
  showInfoDialog(
      "Parting with Matt", "Well then, you're ready to go. Good luck! You have a long and difficult journey ahead of you."
  );
  if (HALT) return;
  state.stage = STATE_STAGE_START;
  setActivity("On the trail");
  autoSave();
  screen_trail();
}

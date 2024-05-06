#include <stdlib.h>

#include "blackjack/card.h"
#include "utils.h"

const char CARD_TYPES[][3] = {
	"A", "2", "3", "4", "5", "6", "7", "8", "9", "10", "J", "Q", "K"
};
const char CARD_SUITS[][4] = { "\u2666", "\u2660", "\u2665", "\u2663" };
const char CARD_BACKS[][2][16] = {
	{"\33[36;44m"
	"/@",
	"@/"},
	{"\33[1;31;41m"
	"::",
	"::"},
	{"\33[30;43m"
	").",
	"'("},
	{"\33[1;37;45m"
	"][",
	"]["}
};

// TODO: wrapping
void drawCard(Card card, byte x, byte y)
{
	setCursorPos(x, y);
	putsn(card.suit == SUIT_HEARTS || card.suit == SUIT_DIAMONDS ? SUIT_COLOR_RED : SUIT_COLOR_BLACK);
	if (CARD_TYPES[card.type][1] == 0) putchar(' ');
	putsn(CARD_TYPES[card.type]);
	setCursorPos(x, y + 1);

	putsn(CARD_SUITS[card.suit]);
	putsn(" " ANSI_COLOR_RESET);
}

void drawCardBack(byte x, byte y, CardBackStyle style)
{
	setCursorPos(x, y);
	putsn(CARD_BACKS[style][0]);
	setCursorPos(x, y + 1);
	putsn(CARD_BACKS[style][1]);
	putsn(ANSI_COLOR_RESET);
}

CardBackStyle chooseCardBackStyle(void)
{
	return rand() % countof(CARD_BACKS);
}
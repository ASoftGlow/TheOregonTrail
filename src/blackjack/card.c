#include <wchar.h>

#include "blackjack/card.h"
#include "utils.h"

static const char CARD_TYPES[][3] = {
	"A", "2", "3", "4", "5", "6", "7", "8", "9", "10", "J", "Q", "K"
};
static const wchar_t CARD_SUITS[] = { 0x2666, 0x2660, 0x2665, 0x2663 };

// TODO: wrapping
void drawCard(Card card, byte x, byte y)
{
	setCursorPos(x, y);
	putsn(card.suit == SUIT_HEARTS || card.suit == SUIT_DIAMONDS ? "\33[31;47m" : "\33[30;47m");
	if (CARD_TYPES[card.type][1] == 0) putchar(' ');
	putsn(CARD_TYPES[card.type]);
	setCursorPos(x, y + 1);

	//putwchar(CARD_SUITS[card.suit]);
	wprintf(L"%lc", CARD_SUITS[card.suit]);
	putsn(" " ANSI_COLOR_RESET);
}

void drawCardBack(byte x, byte y)
{
	setCursorPos(x, y);
	putsn("\33[36;44m" "/@");
	setCursorPos(x, y + 1);
	putsn("@/" ANSI_COLOR_RESET);
}
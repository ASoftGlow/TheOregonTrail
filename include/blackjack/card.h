#include "base.h"

#define CARD_ACE 0
#define CARD_2 1
#define CARD_3 2
#define CARD_4 3
#define CARD_5 4
#define CARD_6 5
#define CARD_7 6
#define CARD_8 7
#define CARD_9 8
#define CARD_10 9
#define CARD_JACK 10
#define CARD_QUEEN 11
#define CARD_KING 12

#define SUIT_CLUBS 0
#define SUIT_DIAMONDS 1
#define SUIT_HEARTS 2
#define SUIT_SPADES 3

#define DECK_SIZE 52
#define LARGEST_HAND 11

typedef struct
{
	byte type, suit;
} Card;

void drawCard(Card card, byte x, byte y);
void drawCardBack(byte x, byte y);
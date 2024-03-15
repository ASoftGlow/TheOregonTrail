#include "blackjack/game.h"
#include "blackjack/card.h"
#include "utils.h"
#include "input.h"
#include "tui.h"
#include "state.h"

#define PTD 2
#define PTH 5
#define PL 2

static const byte CARD_POINTS[] = {
	1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 10, 10, 10
};

Card deck[DECK_SIZE] = {
	{0,0}, {1,0}, {2,0}, {3,0}, {4,0}, {5,0}, {6,0}, {7,0}, {8,0}, {9,0}, {10,0}, {11,0}, {12,0},
	{0,1}, {1,1}, {2,1}, {3,1}, {4,1}, {5,1}, {6,1}, {7,1}, {8,1}, {9,1}, {10,1}, {11,1}, {12,1},
	{0,2}, {1,2}, {2,2}, {3,2}, {4,2}, {5,2}, {6,2}, {7,2}, {8,2}, {9,2}, {10,2}, {11,2}, {12,2},
	{0,3}, {1,3}, {2,3}, {3,3}, {4,3}, {5,3}, {6,3}, {7,3}, {8,3}, {9,3}, {10,3}, {11,3}, {12,3}
};
byte deck_i;

Card hand[LARGEST_HAND];
byte hand_i;
byte points;
bool has_ace;
byte doubled_down;
Card dealer_hand[LARGEST_HAND];
byte dealer_hand_i;
byte dealer_points;

static void dealCard(void)
{
	drawCard(hand[++hand_i] = deck[++deck_i], PL + 3 * hand_i, PTH);
	points += CARD_POINTS[hand[hand_i].type];
	if (hand[hand_i].type == CARD_ACE) has_ace = 1;
	fflush(stdout);
}

static void dealCardToDealer(bool hidden)
{
	tot_sleep(300);
	dealer_hand[++dealer_hand_i] = deck[++deck_i];
	if (hidden)
		drawCardBack(PL + 3 * dealer_hand_i, PTD);
	else
		drawCard(dealer_hand[dealer_hand_i], PL + 3 * dealer_hand_i, PTD);
	if (dealer_hand[dealer_hand_i].type == CARD_ACE)
		dealer_points += 11;
	else
		dealer_points += CARD_POINTS[dealer_hand[dealer_hand_i].type];
	fflush(stdout);
}

static void grayOptions(void)
{
	putsn(ANSI_COLOR_GRAY ANSI_CURSOR_POS("0", "9") "H - Hit\nS - Stand\nD - Double down\nQ - Quit" ANSI_COLOR_RESET);
	fflush(stdout);
}

enum BlackjackGameResult playBlackjack(void)
{
	float bet = 10.2f;
	deck_i = 0;
	hand_i = -1;
	points = 0;
	has_ace = 0;
	doubled_down = 0;
	dealer_hand_i = -1;
	dealer_points = 0;
	shuffle(deck, DECK_SIZE);

	clearStdout();
	putsn(ANSI_CURSOR_HIDE);
	printf("Bet: $%.2f", bet);
	putsn(ANSI_CURSOR_POS("0", "9") "H - Hit\nS - Stand\nD - Double down\nQ - Quit");
	if (state.money < bet * 2)
		putsn(ANSI_CURSOR_POS("0", "11") ANSI_COLOR_GRAY "D - Double down" ANSI_COLOR_RESET);

	// deal cards
	dealCard();
	dealCardToDealer(1);
	tot_sleep(300);
	dealCard();
	dealCardToDealer(0);

	if (hand[0].type == hand[1].type)
	{
		putsn(ANSI_CURSOR_POS("0", "13") "P - Split");
		fflush(stdout);
	}

input_loop:
	switch (getKeyInput())
	{
	case 'H':
	case 'h':
		if (doubled_down == 1)
		{
			doubled_down = 2;
			putsn(ANSI_COLOR_GRAY ANSI_CURSOR_POS("0", "9") "H - Hit" ANSI_COLOR_RESET);
			fflush(stdout);
		}
		else if (doubled_down == 2)
			goto input_loop;

		dealCard();

		if (points > 21) // bust
			goto lost;
		goto input_loop;

	case 's':
	case 'S':
		grayOptions();
		break;

	case 'd':
	case 'D':
		if (state.money < bet * 2 || doubled_down) goto input_loop;
		doubled_down = 1;
		bet *= 2;
		printf(ANSI_CURSOR_POS("0", "0") "Bet: $%.2f", bet);
		putsn(ANSI_CURSOR_POS("0", "11") ANSI_COLOR_GRAY "D - Double down" ANSI_COLOR_RESET);
		fflush(stdout);
		goto input_loop;

	case KEY_QUIT:
	case KEY_QUIT_ALL:
	case 'q':
	case 'Q':
		return BLACKJACK_GAME_QUIT;

	case 'p':
	case 'P':
		if (hand[0].type == hand[1].type)
		{
			// TODO splitting
		}

	default:
		goto input_loop;
	}

	// reveal first card
	drawCard(dealer_hand[0], PL, PTD);
	fflush(stdout);
	tot_sleep(300);

	while (1)
	{
		if (dealer_points < 17)
		{
			dealCardToDealer(0);
			if (dealer_points > 21) // bust
				goto won;
		}
		else
		{
			if (has_ace && points <= 11) points += 10; // count ace as 11
			if (points == dealer_points)
			{
				grayOptions();
				tot_sleep(1000);
				return BLACKJACK_GAME_PUSH;
			}
			if (points < dealer_points)
			{
			lost:
				state.money -= bet;
				grayOptions();
				tot_sleep(1000);
				return BLACKJACK_GAME_LOST;
			}
			if (points == 21)
			{
				state.money += bet * 1.5f; // fixme
				grayOptions();
				tot_sleep(1000);
				return BLACKJACK_GAME_BLACKJACK;
			}
		won:
			state.money += bet;
			grayOptions();
			tot_sleep(800);
			return BLACKJACK_GAME_WON;
		}
	}
}

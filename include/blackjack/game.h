enum BlackjackGameResult
{
	BLACKJACK_GAME_QUIT = -2,
	BLACKJACK_GAME_LOST = -1,
	BLACKJACK_GAME_PUSH,
	BLACKJACK_GAME_WON,
	BLACKJACK_GAME_BLACKJACK
};

enum BlackjackGameResult playBlackjack(void);
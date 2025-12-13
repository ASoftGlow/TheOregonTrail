enum BlackjackResult
{
  BLACKJACK_RESULT_QUIT = -4,
  BLACKJACK_RESULT_LOST = -3,
  BLACKJACK_RESULT_LOST_BLACKJACK = -2,
  BLACKJACK_RESULT_LOST_BUST = -1,
  BLACKJACK_RESULT_PUSH,
  BLACKJACK_RESULT_WON,
  BLACKJACK_RESULT_WON_BUST,
  BLACKJACK_RESULT_WON_BLACKJACK
};

enum BlackjackResult playBlackjack(float bet);
const char* getBlackjackResult(enum BlackjackResult result);

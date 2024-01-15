#pragma once
#include <stdio.h>

#include "base.h"
#include "ansi_codes.h"

// puts without newline
static inline int putsn(const char* str)
{
	return fputs(str, stdout);
}

// strcat but single char
char* _strcat_ch(char* dst, const char src);

inline void clearStdout(void)
{
	putsn(ANSI_CURSOR_ZERO "\033[2J");
}

void setCursorPos(byte x, byte y);
#ifdef _WIN32
void setupConsoleWIN();
#endif
// strlen but ignores ANSI escape codes
size_t strlen_iae(const char* str);
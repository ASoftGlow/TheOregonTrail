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

static inline void clearStdout(void)
{
	putsn(ANSI_CURSOR_ZERO "\033[2J");
}

void setCursorPos(byte x, byte y);

// strlen but ignores ANSI escape codes
size_t _strlen_iae(const char* str);
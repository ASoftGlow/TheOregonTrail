#pragma once
#include "base.h"

// puts without newline
static inline int putsn(const char* str)
{
	return fputs(str, stdout);
}

// strcat but single char
char* strcat_ch(char* dst, const char src);

void clearStdout(void);
void setCursorPos(byte x, byte y);
#ifdef _WIN32
void setupConsoleWIN();
#endif
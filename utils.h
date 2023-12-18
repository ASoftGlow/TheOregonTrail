#pragma once
#include "base.h"

// puts without newline
inline int puts_n(const char* str)
{
	return fputs(str, stdout);
}

// strcat but single char
char* strcat_ch(char* dst, const char src);

void clear_stdout(void);
void set_cursor_pos(byte x, byte y);
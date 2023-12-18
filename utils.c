#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

#include "utils.h"

char* strcat_ch(char* dst, const char src)
{
	char* cp = dst;

	while (*cp) cp++;                   /* find end of dst */

	*cp = src;       /* Copy src to end of dst */
	*++cp = 0;

	return dst;                  /* return dst */
}

void clear_stdout(void)
{
	puts_n("\033[1;1H" "\033[2J");
}

void set_cursor_pos(byte x, byte y)
{
	const byte dy = ++y / (byte)10;
	y -= dy * 10;
	const byte dx = ++x / (byte)10;
	x -= dx * 10;
	puts_n("\033[");
	putchar('0' + dy);
	putchar('0' + y);
	putchar(';');
	putchar('0' + dx);
	putchar('0' + x);
	putchar('H');
}
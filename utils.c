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

size_t strlen_iae(const char* str)
{
	size_t len = 0, pos = 0;

	while (1)
	{
		if (str[pos] == 0) break;
		if (str[pos] == 27)
		{
			// ignore ANSI escape codes
			while (!isalpha(str[++pos]));
		}
		else
			len++;
		pos++;
	}
	return len;
}

void clear_stdout(void)
{
	puts_n("\033[1;1H" "\033[2J");
}
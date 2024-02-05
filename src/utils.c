#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

#include "utils.h"

char* _strcat_ch(char* dst, const char src)
{
	char* cp = dst;

	while (*cp) cp++;                   /* find end of dst */

	*cp = src;       /* Copy src to end of dst */
	*++cp = 0;

	return dst;                  /* return dst */
}

// functional for 2 digits
void setCursorPos(byte x, byte y)
{
	const byte dy = ++y / (byte)10;
	y -= dy * 10;
	const byte dx = ++x / (byte)10;
	x -= dx * 10;
	putsn("\033[");
	putchar('0' + dy);
	putchar('0' + y);
	putchar(';');
	putchar('0' + dx);
	putchar('0' + x);
	putchar('H');
}

size_t _strlen_iae(const char* str)
{
	size_t len = 0, pos = 0;

	while (1)
	{
		if (str[pos] == 0) break;
		if (str[pos] == 27)
		{
			while (!isalpha(str[++pos]));
		}
		else ++len;
		++pos;
	}
	return len;
}

// https://benpfaff.org/writings/clc/shuffle.html
void shuffle(short* array, int size)
{
	for (int i = 0; i < size - 1; i++)
	{
		int j = i + rand() / (RAND_MAX / (size - i) + 1);
		short t = array[j];
		array[j] = array[i];
		array[i] = t;
	}
}
#include <stdlib.h>
#ifdef _WIN32
#include <conio.h>
#else
#include <curses.h>
#endif

#include "input.h"

int qgetch(enum QKeyType* key_type)
{
	int key = _getch();
	if (key == 3 || key == 4) exit(-1);
	if (key == 0 || key == 224) { // if the first value is esc
		*key_type = QKEY_TYPE_ARROW;
		return _getch();
	}

	*key_type = QKEY_TYPE_NORMAL;
	return key;
}

void waitForKey(const int key)
{
	enum QKeyType type;
	while (1)
	{
		if (qgetch(&type) == key && type == QKEY_TYPE_NORMAL) break;
	}
}

int getNumber(int start, int end, const vQKeyCallback key_callback)
{
	return vgetNumber(start, end, key_callback);
}

int vgetNumber(int start, int end, const vQKeyCallback key_callback, ...)
{
	va_list argptr;
	va_start(argptr, key_callback);

	int num = 0;
	int factor = 1;
	int i = 0;
	int buffer[16];

	enum QKeyType type;
	int key;
	while (1)
	{
		key = qgetch(&type);
		if (key_callback && !(*key_callback)(key, type, argptr)) continue;
		if (key == '\r') break;

		if (type == QKEY_TYPE_NORMAL)
		{
			switch (key)
			{
			case '\b':
				if (!i) break;
				factor /= 10;
				num -= buffer[--i] * factor;
				puts_n("\b \b");
				break;

			default:
				const int digit = key - '0';
				if ((end > 9 || (digit >= start && digit <= end)) && (num + digit * factor <= end))
				{
					num += digit * factor;
					factor *= 10;
					putchar(key);
					buffer[i++] = digit;
				}
				break;
			}

		}
	}
	return num;
}

void getString(char* buffer, int min_len, int max_len, const vQKeyCallback key_callback)
{
	vgetString(buffer, min_len, max_len, key_callback);
}

void vgetString(char* buffer, int min_len, int max_len, const vQKeyCallback key_callback, ...)
{
	va_list argptr;
	va_start(argptr, key_callback);

	int i = 0;

	enum QKeyType type;
	int key;
	while (1)
	{
		key = qgetch(&type);
		if (key_callback && !(*key_callback)(key, type, argptr)) continue;
		if (key == '\r' && i >= min_len) break;

		if (type == QKEY_TYPE_NORMAL)
		{
			switch (key)
			{
			case '\b':
				if (!i) break;
				--i;
				puts_n("\b \b");
				break;

			default:
				if (isalpha(key) && i < max_len)
				{
					buffer[i++] = putchar(key);
				}
				break;
			}

		}
	}
	buffer[i] = 0;
}
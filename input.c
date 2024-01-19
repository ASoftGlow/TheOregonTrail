#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#ifdef _WIN32
#include <conio.h>
#else
#include <curses.h>
#endif
#include <errno.h>

#include "input.h"
#include "utils.h"
#include "ansi_codes.h"

bool IS_TTY;
bool escape_combo = 0;

int getKeyInput(enum QKeyType* key_type)
{
	int key = _getch();
	switch (key)
	{
	case 3:
	case 4:
		*key_type = QKEY_TYPE_QUIT;
		errno = ENOENT;
		return 1;

	case ESCAPE_CHAR:
		if (!escape_combo)
		{
			escape_combo = 1;
			return getKeyInput(key_type);
		}
		escape_combo = 0;
		*key_type = QKEY_TYPE_QUIT;
		return 0;

	case 0:
	case 224:
		escape_combo = 0;
		*key_type = QKEY_TYPE_ARROW;
		return _getch();

	default:
		escape_combo = 0;
		*key_type = QKEY_TYPE_NORMAL;
		return key;
	}
}

void waitForKey(const int key)
{
	enum QKeyType type;
	while (1)
	{
		if ((getKeyInput(&type) == key && type == QKEY_TYPE_NORMAL) || type == QKEY_TYPE_QUIT) break;
	}
}

// @returns a negative number if cancelled
int getNumberInput(unsigned start, unsigned end, bool erase, const QKeyCallback key_callback, ...)
{
	va_list argptr;
	va_start(argptr, key_callback);

	unsigned num = 0;
	byte i = 0;
	unsigned buffer[8];

	enum QKeyType type;
	int key;
	while (1)
	{
		key = getKeyInput(&type);
		if (key_callback)
		{
			enum QKeyCallbackReturn result = ((*key_callback)(key, type, argptr));
			if (result == QKEY_CALLBACK_RETURN_IGNORE) continue;
			if (result == QKEY_CALLBACK_RETURN_END)
			{
				// overwrite old text
				if (erase) while (i--) putsn("\b \b");
				return -1;
			}
		}
		// enter pressed
		if (key == '\r' && i) break;

		if (type == QKEY_TYPE_NORMAL)
		{
			switch (key)
			{
			case '\b':
				if (!i) break;
				num -= buffer[--i];
				num /= 10;
				putsn("\b \b");
				fflush(stdout);
				break;

			default:
				if (key < '0' || key > '9') break;
				const unsigned digit = key - '0';
				if ((digit == 0 || i == 1) && (i && !num)) break;
				if (end == 0
					? digit >= start
					: (end > 9 || (digit >= start && digit <= end)) && (num * 10 + digit <= end))
				{
					num *= 10;
					num += digit;
					putchar(key);
					buffer[i++] = digit;
					fflush(stdout);
				}
				break;
			}

		}
		else if (type == QKEY_TYPE_QUIT)
		{
			return -1;
		}
	}
	if (erase) while (i--) putsn("\b \b");
	return num;
}

bool getStringInput(char* buffer, int min_len, int max_len, const QKeyCallback key_callback, ...)
{
	va_list argptr;
	va_start(argptr, key_callback);

	int i = 0;

	enum QKeyType type;
	int key;
	while (1)
	{
		key = getKeyInput(&type);
		if (key_callback && !(*key_callback)(key, type, argptr)) continue;
		if (key == '\r' && i >= min_len) break;

		if (type == QKEY_TYPE_NORMAL)
		{
			switch (key)
			{
			case '\b':
				if (!i) break;
				--i;
				putsn("\b \b");
				fflush(stdout);
				break;

			default:
				if ((isalpha(key) || key == ' ' || key == '\'') && i < max_len)
				{
					buffer[i++] = putchar(key);
					fflush(stdout);
				}
				break;
			}
		}
		else if (type == QKEY_TYPE_QUIT)
		{
			return 1;
		}
	}
	buffer[i] = 0;
	return 0;
}
#include <stdlib.h>
#include <stdio.h>
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

	case 0:
	case 224:
		escape_combo = 0;
		*key_type = QKEY_TYPE_ARROW;
		return _getch();

	case ESCAPE_CHAR:
		if (escape_combo)
		{
			escape_combo = 0;
			*key_type = QKEY_TYPE_QUIT;
			return 0;
		}
		escape_combo = 1;
		goto skip;

	default:
		escape_combo = 0;
	skip:
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
		if (key_callback)
		{
			const enum QKeyCallbackReturn ret = key_callback(key, type, argptr);
			if (ret == QKEY_CALLBACK_RETURN_IGNORE) continue;
			if (ret == QKEY_CALLBACK_RETURN_END) break;
		}

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
				if (i < max_len)
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

static enum QKeyCallbackReturn booleanInputCallback(unsigned key, enum QKeyType type, va_list args)
{
	const QKeyCallback callback = va_arg(args, const QKeyCallback);

	if (type == QKEY_TYPE_NORMAL)
		switch (key)
		{
		case 'y':
		case 'Y':
		case 'n':
		case 'N':
		case '0':
		case '1':
		case 't':
		case 'T':
		case 'f':
		case 'F':
			//return QKEY_CALLBACK_RETURN_END;

		case '\b':
		case '\r':
			return QKEY_CALLBACK_RETURN_NORMAL;

		default:
			if (callback)
			{
				const enum QKeyCallbackReturn ret = callback(key, type, args);
				if (ret == QKEY_CALLBACK_RETURN_NORMAL) return QKEY_CALLBACK_RETURN_IGNORE;
				return ret;
			}
			return QKEY_CALLBACK_RETURN_IGNORE;
		}
	else if (type == QKEY_TYPE_QUIT) return QKEY_CALLBACK_RETURN_END;
	return QKEY_CALLBACK_RETURN_NORMAL;
}

bool getBooleanInput(const QKeyCallback key_callback)
{
	char key[2];
	getStringInput(key, 1, 1, &booleanInputCallback, key_callback);
	return key[0] == 'y' || key[0] == 'Y' || key[0] == '1' || key[0] == 't' || key[0] == 'T';
}

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#ifdef _WIN32
#include <conio.h>
#define tot_getchar() _getch()
#else
#define tot_getchar() getchar()
#endif

#include "input.h"
#include "utils.h"
#include "ansi_codes.h"

bool IS_TTY;
bool escape_combo = 0;

#ifdef __UNIX__
#include <sys/ioctl.h>
#include <unistd.h>
Coord getScreenSize(void)
{
	Coord size = { 0,0 };
#ifdef TIOCGSIZE
	struct ttysize ts;
	ioctl(STDIN_FILENO, TIOCGSIZE, &ts);
	size.x = ts.ts_cols;
	size.y = ts.ts_lines;
#elif defined(TIOCGWINSZ)
	struct winsize ts;
	ioctl(STDIN_FILENO, TIOCGWINSZ, &ts);
	size.x = ts.ws_col;
	size.y = ts.ws_row;
#endif
	return size;
}

#elif _WIN32
#include <windows.h>
Coord getScreenSize(void)
{
	Coord size = { 0,0 };
	CONSOLE_SCREEN_BUFFER_INFO csbi;

	GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
	size.x = csbi.srWindow.Right - csbi.srWindow.Left - 1;
	size.y = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
	return size;
}
#endif

int getKeyInput(void)
{
	// https://stackoverflow.com/a/912796
	int key = tot_getchar();

	// EOF
	if (key < 0)
	{
		return KEY_QUIT_ALL;
	}

	switch (key)
	{
	case 3:
	case 4:
		errno = ECANCELED;
		return KEY_QUIT_ALL;

	case 0:
	case 224:
		escape_combo = 0;
		switch (tot_getchar())
		{
		case 72:
			return KEY_ARROW_UP;
		case 80:
			return KEY_ARROW_DOWN;
		case 77:
			return KEY_ARROW_RIGHT;
		case 75:
			return KEY_ARROW_LEFT;
		case 73:
			return KEY_PAGE_UP;
		case 81:
			return KEY_PAGE_DOWN;

		default:
			return 0;
		}

	case ESC_CHAR:
		if (escape_combo)
		{
			escape_combo = 0;
			return KEY_QUIT;
		}
		escape_combo = 1;
		return key;

	default:
		escape_combo = 0;
		return key;
	}
}

void waitForKey(int key)
{
	while (1)
	{
		const int ikey = getKeyInput();
		if (ikey == key || KEY_IS_TERMINATING(ikey)) break;
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

	int key;
	while (1)
	{
		key = getKeyInput();
		if (key_callback)
		{
			enum QKeyCallbackReturn result = ((*key_callback)(key, argptr));
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
		if (KEY_IS_TERMINATING(key)) return -1;

		switch (key)
		{
		case '\b':
			if (!i) break;
			num -= buffer[--i];
			num /= 10;
			putsn("\b \b");
			fflush(stdout);
			break;

		case DEL_CHAR:
			if (!i) break;
			num = 0;
			do putsn("\b \b");
			while (--i);
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
	if (erase) while (i--) putsn("\b \b");
	return num;
}

bool getStringInput(char* buffer, int min_len, int max_len, const QKeyCallback key_callback, ...)
{
	va_list argptr;
	va_start(argptr, key_callback);

	int i = 0;

	int key;
	while (1)
	{
		key = getKeyInput();
		if (key_callback)
		{
			const enum QKeyCallbackReturn ret = key_callback(key, argptr);
			if (ret == QKEY_CALLBACK_RETURN_IGNORE) continue;
			if (ret == QKEY_CALLBACK_RETURN_END) break;
		}

		if (key == '\r' && i >= min_len) break;
		if (KEY_IS_TERMINATING(key)) return -1;

		switch (key)
		{
		case '\b':
			if (!i) break;
			--i;
			putsn("\b \b");
			fflush(stdout);
			break;

		case DEL_CHAR:
			if (!i) break;
			do putsn("\b \b");
			while (--i);
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
	buffer[i] = 0;
	return 0;
}

static enum QKeyCallbackReturn booleanInputCallback(int key, va_list args)
{
	const QKeyCallback callback = va_arg(args, const QKeyCallback);

	if (KEY_IS_TERMINATING(key)) return QKEY_CALLBACK_RETURN_END;
	if (KEY_IS_ARROWS(key)) return QKEY_CALLBACK_RETURN_NORMAL;

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

	case '\b':
	case '\r':
	case DEL_CHAR:
		return QKEY_CALLBACK_RETURN_NORMAL;

	default:
		if (callback)
		{
			const enum QKeyCallbackReturn ret = callback(key, args);
			if (ret == QKEY_CALLBACK_RETURN_NORMAL) return QKEY_CALLBACK_RETURN_IGNORE;
			return ret;
		}
		return QKEY_CALLBACK_RETURN_IGNORE;
	}
}

bool getBooleanInput(const QKeyCallback key_callback)
{
	char key[2];
	getStringInput(key, 1, 1, &booleanInputCallback, key_callback);
	return key[0] == 'y' || key[0] == 'Y' || key[0] == '1' || key[0] == 't' || key[0] == 'T';
}

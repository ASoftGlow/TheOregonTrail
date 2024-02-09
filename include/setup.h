#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <locale.h>
#ifndef TOT_TTY
#include "nfd.h"
#endif

#include "utils.h"
#include "ansi_codes.h"

#ifdef _WIN32
#include <Windows.h>
static inline void setupWin(void)
{
	IS_TTY = 0;
	// enable ANSI escape codes
	HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
	DWORD dwMode = 0;
	GetConsoleMode(hOut, &dwMode);
	dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
	SetConsoleMode(hOut, dwMode);
#ifndef TOT_ASCII
	SetConsoleOutputCP(CP_UTF8);
#endif
}

#elif __APPLE__
#include <unistd.h>
static inline void setupMacOS(void)
{
	IS_TTY = isatty(fileno(stdout));
}

#else
#include <termios.h>
#include <unistd.h>

struct termios oldt;
struct termios newtw, newti;

static inline void setupLinux(void)
{
	IS_TTY = getenv("DISPLAY") == 0;

	tcgetattr(STDIN_FILENO, &oldt);
	newtw = oldt;
	newtw.c_lflag &= ~(ICANON | ECHO);
	newti = newtw;
	newti.c_cc[VTIME] = 0;
	newti.c_cc[VMIN] = 0;
	tcsetattr(STDIN_FILENO, TCSANOW, &newtw);
}
#endif


void setup(void)
{
#ifdef _WIN32
	setupWin();
#elif __APPLE__
	setupMacOS();
#else
	setupLinux();
#endif
#ifdef TOT_TTY
	IS_TTY = 1;
#endif

	if (!IS_TTY)
	{
		if (!NFD_Init())
		{
			// fallback to tty
			IS_TTY = 1;
		}
	}

	srand((unsigned)time(NULL));

	// make stdout fully buffered
	setvbuf(stdout, NULL, _IOFBF, (size_t)1 << 12);

	// style console
	putsn(
		ANSI_CURSOR_STYLE_UNDERLINE ANSI_CURSOR_SHOW ANSI_WINDOW_TITLE("Oregon Trail") ANSI_NO_WRAP
	);

	// unicode
	setlocale(LC_CTYPE, "");
}

void setdown(void)
{
	putsn(ANSI_CURSOR_RESTORE ANSI_COLOR_RESET ANSI_CURSOR_STYLE_DEFAULT ANSI_CURSOR_SHOW ANSI_WRAP);
	fflush(stdout);

#ifndef _WIN32
	tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
#endif
	NFD_Quit();
}
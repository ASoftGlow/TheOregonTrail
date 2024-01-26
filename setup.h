#pragma once
#include <stdio.h>

#include "ansi_codes.h"
// TODO: allow reading cursor position

#ifdef _WIN32
#include <Windows.h>
static void setupWin(void)
{
	IS_TTY = 0;
	// enable ANSI escape codes
	HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
	DWORD dwMode = 0;
	GetConsoleMode(hOut, &dwMode);
	dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
	SetConsoleMode(hOut, dwMode);
	SetConsoleOutputCP(437);
}

#elif __APPLE__
#include <unistd.h>
static void setupMacOS(void)
{
	IS_TTY = isatty(fileno(stdout));
}

#else
static void setupUnix(void)
{
	IS_TTY = getenv("DISPLAY") != 0;
}
#endif


void setup(void)
{
#ifdef _WIN32
	setupWin();
#elif __APPLE__
	setupMacOS();
#else
	setupUnix();
#endif

	// make stdout fully buffered
	setvbuf(stdout, NULL, _IOFBF, (size_t)1 << 12);

	// style console
	putsn(
		ANSI_CURSOR_STYLE_UNDERLINE ANSI_CURSOR_SHOW ANSI_WINDOW_TITLE("Oregon Trail")
		ANSI_WINDOW_SIZE(TOKENXSTR(SCREEN_WIDTH + 2), "") ANSI_NO_WRAP
	);
}
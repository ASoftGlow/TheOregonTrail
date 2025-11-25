#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#ifndef TOT_TTY
#include "nfd.h"
#endif
#ifndef TOT_MUTE
#include "music.h"
#endif
#ifdef TOT_DISCORD
#include "discord.h"
#endif

#include "ansi_codes.h"
#include "input.h"
#include "state.h"
#include "utils.h"

#ifdef _WIN32
#include <Windows.h>

static inline void
setupWin(void)
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

static inline void
setupMacOS(void)
{
  IS_TTY = isatty(fileno(stdout));
}

#else
#include <signal.h>
#include <termios.h>
#include <unistd.h>

struct termios oldt;
struct termios newtw, newti;

static void
handleSig(int sig)
{
  switch (sig)
  {
  case SIGWINCH:
    if (settings.auto_screen_size)
      // This function is NOT signal safe, however, the chances of
      // the user resizing the window while it is being drawn is low.
      updateScreenSize();
    break;

    // case SIGINT:
    // 	puts("sigint");
    // 	fflush(stdout);
    // 	break;

  default: break;
  }
}

static inline void
setupLinux(void)
{
  IS_TTY = getenv("DISPLAY") == 0;

  tcgetattr(STDIN_FILENO, &oldt);
  newtw = oldt;
  newtw.c_lflag &= ~(ICANON | ECHO);
  newti = newtw;
  newti.c_cc[VTIME] = 0;
  newti.c_cc[VMIN] = 0;
  tcsetattr(STDIN_FILENO, TCSANOW, &newtw);

  signal(SIGWINCH, &handleSig);
  signal(SIGINT, &handleSig);
}
#endif

Coord original_screen_size;

bool
setup(void)
{
#ifdef _WIN32
  setupWin();
#elif __APPLE__
  setupMacOS();
#else
  setupLinux();
#endif

  srand((unsigned)time(NULL));

  // make stdout fully buffered
  setvbuf(stdout, NULL, _IOFBF, (size_t)1 << 12);

  // style console
  putsn(ANSI_SB_ALT ANSI_CURSOR_STYLE_UNDERLINE ANSI_CURSOR_SHOW ANSI_WINDOW_TITLE("Oregon Trail") ANSI_NO_WRAP);

  original_screen_size = getScreenSize();

#ifdef TOT_TTY
  IS_TTY = 1;
#else
  if (!IS_TTY)
  {
    if (!NFD_Init())
    {
      // fallback to tty
      IS_TTY = 1;
    }
  }
#endif

#ifndef TOT_MUTE
  if (music_setup()) return 1;
#endif

  return 0;
}

bool
post_setup(void)
{
#ifdef TOT_DISCORD
  if (settings.discord_rp)
    if (discord_setup()) return 1;
#endif

  return 0;
}

void
setdown(void)
{
  putsn(ANSI_SB_MAIN ANSI_COLOR_RESET ANSI_CURSOR_STYLE_DEFAULT ANSI_CURSOR_SHOW ANSI_WRAP);
  if (IS_TTY) clearStdout();

#ifndef _WIN32
  tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
#endif
#ifndef TOT_TTY
  NFD_Quit();
#endif
#ifndef TOT_MUTE
  music_setdown();
#endif
#ifdef TOT_DISCORD
  discord_setdown();
#endif
  enableResizing();
  setScreenSize(original_screen_size.x, original_screen_size.y);

  if (EXIT_MSG) puts(EXIT_MSG);
  fflush(stdout);
}

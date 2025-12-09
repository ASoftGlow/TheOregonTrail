#include <stdio.h>
#include <stdlib.h>
#ifdef _WIN32
#include <conio.h>
#define tot_getchar() _getch()
#else
#define tot_getchar() getchar()
#endif

#include "input.h"
#include "utils.h"

bool IS_TTY;
bool escape_combo = 0;

void
setScreenSize(int width, int height)
{
  printf("\33[8;%i;%it", height, width);
}

#ifdef _WIN32
#include <windows.h>

Coord
getScreenSize(void)
{
  Coord size = { 0 };
  CONSOLE_SCREEN_BUFFER_INFO csbi;

  GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
  size.x = csbi.srWindow.Right - csbi.srWindow.Left - 1;
  size.y = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
  return size;
}

void
tot_sleep(unsigned long ms)
{
  Sleep(ms);
}

LONG original_style = LONG_MAX;

void
disableResizing()
{
  HWND hWnd = GetConsoleWindow();
  original_style = GetWindowLong(hWnd, GWL_STYLE);
  SetWindowLong(hWnd, GWL_STYLE, original_style & ~(WS_SIZEBOX | WS_MAXIMIZEBOX));
}

void
enableResizing()
{
  if (original_style == LONG_MAX) return;
  HWND hWnd = GetConsoleWindow();
  SetWindowLong(hWnd, GWL_STYLE, original_style);
}

#else
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>
#ifndef __USE_POSIX199309
#define __USE_POSIX199309
#endif
#include <time.h>

Coord
getScreenSize(void)
{
  Coord size = { 0, 0 };
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

void
disableResizing()
{
  // TODO
}

void
enableResizing()
{
  // TODO
}

void
tot_sleep(unsigned long ms)
{
  nanosleep(
      (const struct timespec[]){
          { ms / 1000UL, ms % 1000UL * 1000000UL }
  },
      NULL
  );
}

extern struct termios newtw, newti;

static inline void
enableInputWait(void)
{
  tcsetattr(STDIN_FILENO, TCSANOW, &newtw);
}

static inline void
disableInputWait(void)
{
  tcsetattr(STDIN_FILENO, TCSANOW, &newti);
}
#endif

int
getKeyInput(void)
{
  // https://stackoverflow.com/a/912796
  int key = tot_getchar();

  // EOF
  if (key < 0)
  {
    HALT = HALT_QUIT;
    return KEY_QUIT_ALL;
  }

  switch (key)
  {
  case 3:
  case 4: HALT = HALT_QUIT; return KEY_QUIT_ALL;

  case 0:
  case 224:
    escape_combo = 0;
    switch (tot_getchar())
    {
    case 72: return KEY_ARROW_UP;
    case 80: return KEY_ARROW_DOWN;
    case 77: return KEY_ARROW_RIGHT;
    case 75: return KEY_ARROW_LEFT;
    case 73: return KEY_PAGE_UP;
    case 81: return KEY_PAGE_DOWN;

    default: return 0;
    }

  case ESC_CHAR:
#ifndef _WIN32
    disableInputWait();
    int ret = -1;
    if (getchar() > 0)
    {
      switch (getchar())
      {
      case 'A': ret = KEY_ARROW_UP; break;
      case 'B': ret = KEY_ARROW_DOWN; break;
      case 'C': ret = KEY_ARROW_RIGHT; break;
      case 'D': ret = KEY_ARROW_LEFT; break;
      case 53:  ret = KEY_PAGE_UP; break;
      case 54:  ret = KEY_PAGE_DOWN; break;
      default:  ret = KEY_UNKNOWN; break;
      }
    }
    else clearerr(stdin);
    enableInputWait();
    if (ret > 0) return ret;
#endif
    if (escape_combo)
    {
      escape_combo = 0;
      if (HALT != HALT_DISALLOWED)
      {
        HALT = HALT_GAME;
        return KEY_QUIT;
      }
      return key;
    }
    escape_combo = 1;
    return key;

  default: escape_combo = 0; return key;
  }
}

void
waitForKey(int key)
{
  while (1)
  {
    const int ikey = getKeyInput();
    if (ikey == key || KEY_IS_TERMINATING(ikey)) break;
  }
}

// @returns a negative number if cancelled
int
getNumberInput(unsigned start, unsigned end, bool erase, const QKeyCallback key_callback, ...)
{
  unsigned num = 0;
  byte i = 0;
  unsigned buffer[8] = { 0 };

  int key;
  while (1)
  {
    key = getKeyInput();
    if (KEY_IS_TERMINATING(key)) return -1;
    if (key_callback)
    {
      va_list argptr;
      va_start(argptr, key_callback);
      enum QKeyCallbackReturn result = ((*key_callback)(key, argptr));
      va_end(argptr);
      if (result == QKEY_CALLBACK_RETURN_IGNORE) continue;
      if (result == QKEY_CALLBACK_RETURN_END)
      {
        // overwrite previous char
        if (erase)
          while (i--) putsn("\b \b");
        return -1;
      }
    }
    // enter pressed
    if (key == ETR_CHAR && i) break;

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
      if (end == 0 ? digit >= start : (end > 9 || (digit >= start && digit <= end)) && ((unsigned long)num * 10 + digit <= end))
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
  if (erase)
    while (i--) putsn("\b \b");
  return num;
}

bool
getStringInput(char* buffer, int min_len, int max_len, const QKeyCallback key_callback, ...)
{
  int i = 0;

  int key;
  while (1)
  {
    key = getKeyInput();
    if (KEY_IS_TERMINATING(key)) return -1;
    if (key_callback)
    {
      va_list argptr;
      va_start(argptr, key_callback);
      const enum QKeyCallbackReturn ret = key_callback(key, argptr);
      va_end(argptr);
      if (ret == QKEY_CALLBACK_RETURN_IGNORE) continue;
      if (ret == QKEY_CALLBACK_RETURN_END) break;
    }

    if (key == ETR_CHAR)
    {
      if (i >= min_len) break;
      continue;
    }

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

bool
getWrappedStringInput(char* buffer, byte width, Coord offset, int min_len, int max_len, const QKeyCallback key_callback, ...)
{
  int i = 0;
  int last_break = width;
  int last_break_x = 0, last_break_y = -1;
  int x = -1, y = 0;

  int key;
  while (1)
  {
    key = getKeyInput();
    if (KEY_IS_TERMINATING(key)) return 0;
    if (key_callback)
    {
      va_list argptr;
      va_start(argptr, key_callback);
      const enum QKeyCallbackReturn ret = key_callback(key, argptr);
      va_end(argptr);
      if (ret == QKEY_CALLBACK_RETURN_IGNORE) continue;
      if (ret == QKEY_CALLBACK_RETURN_END) break;
    }
    if (KEY_IS_ARROW(key)) continue;

    if (key == ETR_CHAR)
    {
      if (i >= min_len) break;
      continue;
    }

    switch (key)
    {
    case '\b':
      if (!i) break;
      if (x || !y)
      {
        --i;
        --x;
        putsn("\b \b");
      }
      else
      {
        --i;
        x = width - 1;
        putsn("\b ");
        setCursorPos(offset.x + width, offset.y + --y);
      }
      fflush(stdout);
      break;

    case DEL_CHAR:
      if (!i) break;
      for (int j = 0; j <= y; j++)
      {
        setCursorPos(offset.x, offset.y + j);
        for (int h = 0; h < width; h++) putchar(' ');
      }
      setCursorPos(offset.x, offset.y);
      fflush(stdout);
      i = 0;
      x = 0;
      last_break = 0;
      break;

    case ' ':
      last_break = i;
      last_break_x = x + 2;
      last_break_y = y;

    default:
      if (i < max_len)
      {
        if (x == width - 1)
        {
          if (last_break == i || last_break_y != y)
          {
            setCursorPos(offset.x, offset.y + ++y);
            x = 0;
          }
          else
          {
            int j = x = width - last_break_x;
            setCursorPos(last_break_x, last_break_y);
            while (j--) putchar(' ');
            setCursorPos(offset.x, offset.y + ++y);
            putsn(buffer + last_break + 1);
          }
        }
        else ++x;
        buffer[i++] = putchar(key);
        fflush(stdout);
      }
      break;
    }
  }
  buffer[i] = 0;
  return 0;
}

static enum QKeyCallbackReturn
booleanInputCallback(int key, va_list args)
{
  const QKeyCallback callback = va_arg(args, const QKeyCallback);

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
  case ETR_CHAR:
  case DEL_CHAR: return QKEY_CALLBACK_RETURN_NORMAL;

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

bool
getBooleanInput(const QKeyCallback key_callback)
{
  char key[2];
  getStringInput(key, 1, 1, &booleanInputCallback, key_callback);
  return key[0] == 'y' || key[0] == 'Y' || key[0] == '1' || key[0] == 't' || key[0] == 'T';
}

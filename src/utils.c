#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include "utils.h"

void
setCursorPos(byte x, byte y)
{
  printf("\33[%i;%iH", (int)y + 1, (int)x + 1);
}

char*
_strcat_ch(char* dst, const char src)
{
  char* cp = dst;
  // find end of dst
  while (*cp) cp++;
  // Copy src to end of dst
  *cp = src;
  *++cp = 0;

  return dst;
}

size_t
_strlen_iae(const char* str)
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

void
puts_warn(const char* msg)
{
  printf(ANSI_SB_MAIN "%s" ANSI_SB_ALT, msg);
  fflush(stdout);
}

void
puts_warnf(const char* format, ...)
{
  putsn(ANSI_SB_MAIN);
  va_list argptr;
  va_start(argptr, format);
  vprintf(format, argptr);
  va_end(argptr);
  putsn(ANSI_SB_ALT);
  fflush(stdout);
}

#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include "ansi_codes.h"
#include "utils.h"

void
setCursorPos(byte x, byte y)
{
  printf(ANSI_CURSOR_POS("%i", "%i"), (int)y + 1, (int)x + 1);
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
  size_t len = 0;

  for (size_t pos = 0; str[pos]; pos++)
  {
    if (str[pos] == 27)
    {
      while (!isalpha(str[++pos]));
    }
    else ++len;
  }
  return len;
}

size_t
_strlen_iae_n(const char* str, size_t full_len)
{
  size_t len = 0;

  for (size_t pos = 0; pos < full_len; pos++)
  {
    if (str[pos] == 27)
    {
      while (!isalpha(str[++pos]));
    }
    else ++len;
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

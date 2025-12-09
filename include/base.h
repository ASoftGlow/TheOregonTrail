#pragma once

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>

typedef uint8_t byte;

typedef struct
{
  byte x, y;
} Coord;

enum HaltType
{
  HALT_DISALLOWED = -1,
  HALT_NONE,
  HALT_GAME,
  HALT_QUIT
};

extern enum HaltType HALT;
extern const char* EXIT_MSG;

#ifndef NULL
#define NULL ((void*)0)
#endif
#define countof(a) (sizeof(a) / sizeof(a[0]))
#ifndef min
#define min(a, b) (((a) < (b)) ? (a) : (b))
#define max(a, b) (((a) > (b)) ? (a) : (b))
#endif
#define elvis(v, e)       (v ? v : e)

#define TOKENPASTE(x, y)  x##y
#define TOKENPASTE2(x, y) TOKENPASTE(x, y)
#define TOKENSTR(x)       #x
#define TOKENXSTR(x)      TOKENSTR(x)

#ifdef _WIN32
#define ETR_CHAR '\r'
#ifdef _DEBUG
#define DEBUG
#endif
#else
#define ETR_CHAR '\n'
#ifndef NDEBUG
#define DEBUG
#endif
#endif

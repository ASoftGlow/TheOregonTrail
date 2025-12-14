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
#define MAYBE_UNUSED
#define FALLTHROUGH
/*
 * MSVC doesn't support VLAs (as it's optional in C11), so _alloca is used to allocate
 * memory for one on the stack. It is deprecated for _malloca, but that requires manually
 * freeing the memory afterwards, which defeats the convenience.
 */
#define VLA(type, var, size) type* var = _alloca(sizeof(type) * (size))
#else
#define ETR_CHAR '\n'
#ifndef NDEBUG
#define DEBUG
#define MAYBE_UNUSED __attribute__((unused))
#define FALLTHROUGH  __attribute__((fallthrough))
#define VLA(type, var, size) type var[size]
#endif
#endif

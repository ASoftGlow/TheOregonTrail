#pragma once

#include <stdbool.h>
#include <assert.h>

typedef unsigned char byte;
typedef struct {
	byte x, y;
} Coord;
enum HaltType
{
	HALT_NONE,
	HALT_GAME,
	HALT_QUIT
};

extern enum HaltType HALT;
extern const char* EXIT_MSG;

#ifndef NULL
#define NULL ((void *)0)
#endif
#define countof(a) (sizeof(a)/sizeof(a[0]))
#ifndef min
#define min(a,b) (((a)<(b))?(a):(b))
#define max(a,b) (((a)>(b))?(a):(b))
#endif
#define elvis(v, e) (v ? v : e)

#define TOKENPASTE(x, y) x ## y
#define TOKENPASTE2(x, y) TOKENPASTE(x, y)
#define TOKENSTR(x) #x
#define TOKENXSTR(x) TOKENSTR(x)

#ifdef _WIN32
#define ETR_CHAR '\r'
#else
#define ETR_CHAR '\n'
#endif
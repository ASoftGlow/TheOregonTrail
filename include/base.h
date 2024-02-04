#pragma once

typedef unsigned char byte;
typedef byte bool;
typedef struct {
	byte x, y;
} Coord;
#ifndef NULL
#define NULL ((void *)0)
#endif
#ifndef _countof
#define _countof(a) (sizeof(a)/sizeof(a[0]))
#endif
#ifndef min
#define min(a,b) (((a)<(b))?(a):(b))
#define max(a,b) (((a)>(b))?(a):(b))
#endif
#define elvis(v, e) (v ? v : e)

#define TOKENPASTE(x, y) x ## y
#define TOKENPASTE2(x, y) TOKENPASTE(x, y)
#define TOKENSTR(x) #x
#define TOKENXSTR(x) TOKENSTR(x)
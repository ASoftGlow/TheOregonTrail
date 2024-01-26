#pragma once

typedef unsigned char byte;
typedef byte bool;
typedef struct {
	byte x, y;
} Coord;
#ifndef NULL
#define NULL ((void *)0)
#endif

#define TOKENPASTE(x, y) x ## y
#define TOKENPASTE2(x, y) TOKENPASTE(x, y)
#define TOKENSTR(x) #x
#define TOKENXSTR(x) TOKENSTR(x)
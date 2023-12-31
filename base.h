#pragma once

typedef unsigned char byte;
typedef int bool;
#ifndef NULL
#define NULL ((void *)0)
#endif

#define TOKENPASTE(x, y) x ## y
#define TOKENPASTE2(x, y) TOKENPASTE(x, y)
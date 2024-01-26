#pragma once
#include <stdlib.h>

#include "base.h"
#include "ansi_codes.h"
#include "tui.h"

const Coord MAP_PATH1_COORDS[] = {
	{10,0}, {11,0},{12,0},{13,1},{14,2},{15,3},{16,3},{17,4},{18,4},{19,5},{20,6},{21,5},{23,4},{25,4},{27,2},{29,2},{31,1},{32,0},{33,0},{34,0},{35,0}
};

const Coord* MAP_PATHS[] = { &MAP_PATH1_COORDS[0] };
const short MAP_PATH_LENGTHS[] = { _countof(MAP_PATH1_COORDS) };

#define MAP_WIDTH 53
const char MAP[][MAP_WIDTH] =
{
"1    /\\^  @-\\                        yes this is map!",
"2 /\\         \\^ ^        \\___  jdf djfdkf d f e  e ed",
"3             \\                       ___ ff sdfoiwef",
"4    /\\^       `\\                    yes this is map!",
"5 /\\         iI. \\_.  ,---     jdf djfdkf d      e ed",
"6                  \\_/      \\________ ___ ff sdfoiwef",
"7                   `                abcdt    is map!",
"8 /\\            ^        \\___  jdf djfdkf d f e  e ed",
"9                            ________ ___ ff sdfoiwef"
};
#define MAP_HEIGHT _countof(MAP)
#define MAP_VIEWPORT_WIDTH SCREEN_WIDTH
#define MAP_VIEWPORT_HEIGHT SCREEN_HEIGHT - 2

byte map_paths_order[] = { 0 };

const char DENSITY_INDICATORS[][12] = {
	"x",
	"\33[1m""x""\33[21m",
	"²"
};

#define INDICATOR(ch) ANSI_COLOR_GREEN ch ANSI_COLOR_RESET
const char
* INDICATOR_UP = INDICATOR("^"),
* INDICATOR_DOWN = INDICATOR("v"),
* INDICATOR_LEFT = INDICATOR("<"),
* INDICATOR_RIGHT = INDICATOR(">"),
* INDICATOR_SLANT_LEFT = INDICATOR("\\"),
* INDICATOR_SLANT_RIGHT = INDICATOR("/");
#include "ansi_codes.h"
#include "base.h"

const Coord MAP_PATH1_COORDS[] = {
  { 10, 0 },
  { 11, 0 },
  { 12, 0 },
  { 13, 1 },
  { 14, 2 },
  { 15, 3 },
  { 16, 3 },
  { 17, 4 },
  { 18, 4 },
  { 19, 5 },
  { 20, 6 },
  { 21, 5 },
  { 23, 4 },
  { 25, 4 },
  { 27, 2 },
  { 29, 2 },
  { 31, 1 },
  { 32, 0 },
  { 33, 0 },
  { 34, 0 },
  { 35, 0 }
};

const Coord* MAP_PATHS[] = { &MAP_PATH1_COORDS[0] };
const short MAP_PATH_LENGTHS[] = { countof(MAP_PATH1_COORDS) };

#define MAP_WIDTH 53
const char MAP[][MAP_WIDTH] = {
  "1    /\\^  @-\\                        yes this is map!", "2 /\\         \\^ ^        \\___  jdf djfdkf d f e  e ed",
  "3             \\                       ___ ff sdfoiwef",  "4    /\\^       `\\                    yes this is map!",
  "5 /\\         iI. \\_.  ,---         djfdkf d      e ed", "6                  \\_/      \\________ ___ ff sdfoiwef",
  "7                   `                abcdt    is map!",   "8 /\\            ^        \\___  jdf djfdkf d f e  e ed",
  "9                            ________ ___ ff sdfoiwef",
};
#define MAP_HEIGHT countof(MAP)

byte map_paths_order[] = { 0 };

const char* DENSITY_INDICATORS[] = {
  "x",
  ANSI_BOLD "x" ANSI_NO_BOLD,
  ANSI_BOLD "#" ANSI_NO_BOLD,
};

#define INDICATOR_COLOR ANSI_COLOR_GRAY
#define INDICATOR(ch)   INDICATOR_COLOR ch ANSI_COLOR_RESET
// clang-format off
const char
#ifdef TOT_ASCII
* INDICATOR_UP          = "^",
* INDICATOR_DOWN        = "v",
* INDICATOR_LEFT        = INDICATOR("<"),
* INDICATOR_RIGHT       = INDICATOR(">"),
* INDICATOR_SLANT_LEFT  = " ",
* INDICATOR_SLANT_RIGHT = " ";
#else
* INDICATOR_UP          = "\u2191",
* INDICATOR_DOWN        = "\u2193",
* INDICATOR_LEFT        = INDICATOR("\u2190"),
* INDICATOR_RIGHT       = INDICATOR("\u2192"),
* INDICATOR_SLANT_LEFT  = " ",
* INDICATOR_SLANT_RIGHT = " ";
#endif
// clang-format on

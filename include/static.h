#pragma once
#include "base.h"

#define WAGON_MEMBER_COUNT 4
#define NAME_SIZE          10
#define NAME_PLACEHOLDER   "xxxxxxxxxx"
static_assert(sizeof(NAME_PLACEHOLDER) - 1 == NAME_SIZE, "Name placeholder size is incorrect");
extern const char WAGON_MEMBER_NAMES[4][NAME_SIZE];

extern const char MONTHS[][16];
extern const byte MONTH_LENGTHS[];
extern const char WEATHERS[][8];
extern const char PACES[][16];
extern const char RATIONS[][16];

extern const byte DISEASE_DURATIONS[];
extern const byte DISEASE_CHANCES[];
extern const byte INJURY_DURATIONS[];
extern const byte INJURY_CHANCES[];

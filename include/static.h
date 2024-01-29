#pragma once
#include "base.h"

#define WAGON_MEMBER_COUNT 4
#define NAME_SIZE 10
extern const char WAGON_MEMBER_NAMES[][NAME_SIZE];

extern const char MONTHS[][8];
extern const byte MONTH_LENGTHS[];
extern const char WEATHERS[][8];
extern const char PACES[][8];
extern const char RATIONS[][8];

extern const byte DISEASE_DURATIONS[];
extern const byte DISEASE_CHANCES[];
extern const byte INJURY_DURATIONS[];
extern const byte INJURY_CHANCES[];

const char* getRandomName(void);
#pragma once
#include "base.h"

#define WAGON_MEMBER_COUNT 4
#define NAME_SIZE 10
extern const char WAGON_MEMBER_NAMES[][NAME_SIZE];

extern const char months[][8];

extern const byte disease_durations[];
extern const byte disease_chances[];
extern const byte injury_durations[];
extern const byte injury_chances[];

const char* getRandomName(void);
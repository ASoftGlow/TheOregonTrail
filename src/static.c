#include <stdlib.h>

#include "static.h"

const char WAGON_MEMBER_NAMES[][NAME_SIZE] = { "John", "Sara", "Henry", "Jed" };

const char MONTHS[][8] = {
	"March",
	"April",
	"May",
	"June",
	"July"
};

const byte MONTH_LENGTHS[] = { 30,30,30,30 }; // TODO

const char WEATHERS[][8] = {
	"mild",
	"cold",
	"hot",
	"rain",
	"snow"
};

const char PACES[][8] = {
	"steady"
};

const char RATIONS[][8] = {
	"filling"
};

const byte DISEASE_DURATIONS[] = { 0 };
const byte DISEASE_CHANCES[] = { 0 };
const byte INJURY_DURATIONS[] = { 20, 30 };
const byte INJURY_CHANCES[] = { 20, 30 };

const char* getRandomName(void)
{
	return WAGON_MEMBER_NAMES[rand() % _countof(WAGON_MEMBER_NAMES)];
}
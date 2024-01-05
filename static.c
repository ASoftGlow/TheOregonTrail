#include <stdlib.h>

#include "static.h";

const char WAGON_MEMBER_NAMES[][NAME_SIZE] = { "John", "Sara", "Henry", "Jed" };

const char months[][8] = {
	"March",
	"April",
	"May",
	"June",
	"July"
};

const byte disease_durations[] = { 0 };
const byte disease_chances[] = { 0 };
const byte injury_durations[] = { 20, 30 };
const byte injury_chances[] = { 20, 30 };

const char* getRandomName(void)
{
	return WAGON_MEMBER_NAMES[rand() % _countof(WAGON_MEMBER_NAMES)];
}
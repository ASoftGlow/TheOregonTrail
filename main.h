#pragma once
#include "base.h"

#define WAGON_MEMBER_COUNT 4
#define NAME_SIZE 10
const char WAGON_MEMBER_NAMES[][NAME_SIZE] = { "John", "Sara", "Henry", "Jed" };

enum WagonMemberHealth
{
	HEALTH_GOOD,
	HEALTH_FAIR,
	HEALTH_POOR,
	HEALTH_VERY_POOR
};

enum Disease
{
	DISEASE_NONE
};
const byte disease_durations[] = { 0 };
const byte disease_chances[] = { 0 };

enum Injury
{
	INJURY_NONE,
	INJURY_BROKEN_ARM,
	INJURY_BROKEN_LEG
};
const byte injury_durations[] = { 20, 30 };
const byte injury_chances[] = { 20, 30 };

struct WagonMember
{
	char name[NAME_SIZE + 1];
	enum WagonMemberHealth health;
	enum Disease disease;
	byte disease_duration;
	enum Injury injury;
	byte injury_duration;
};

enum Role
{
	ROLE_BANKER,
	ROLE_CARPENTER,
	ROLE_FARMER
};

void showMain(void);
void showMonth(void);
void showRole(void);
void showStore(void);

const char* getRandomName(void)
{
	return WAGON_MEMBER_NAMES[rand() % _countof(WAGON_MEMBER_NAMES)];
}

struct StoreItem
{
	const char name[16];
	const float price;
	const byte min;
	const byte max;
	byte amount;
};

struct StoreCategory
{
	const char name[12];
	const char* desciption;
	const char* image;
	const struct StoreItem* items;
	const byte items_count;
	float spent;
};
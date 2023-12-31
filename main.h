#pragma once
#include "base.h"

#define WAGON_MEMBER_COUNT 4
#define NAME_SIZE 10
const char WAGON_MEMBER_NAMES[][NAME_SIZE] = { "John", "Sara", "Henry", "Jed" };

const char months[][8] = {
	"March",
	"April",
	"May",
	"June",
	"July"
};

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
Coord drawStore(void);
void workStore(Coord);
void showStore(void);

const char* getRandomName(void)
{
	return WAGON_MEMBER_NAMES[rand() % _countof(WAGON_MEMBER_NAMES)];
}

struct StoreItem
{
	const char name[16];
	const float price;
	const unsigned min;
	const unsigned max;
	unsigned amount;
};

struct StoreCategory
{
	const char name[12];
	const char* desciption;
	const char* image;
	struct StoreItem* items;
	const byte items_count;
	float spent;
};

struct StoreCategory STORE_MATT_CATEGORIES[] = {
	{
		.name = "Oxen",
		.desciption = "There are 2 oxen in a yoke; I recommend at least 3 yoke. I charge $40 a yoke.",
		.image = "o-uu,",
		.items = (struct StoreItem[]) {
			{.name = "yokes", .price = 10.f, .min = 1, .max = 9}
		},
		.items_count = 1
	},
	{
		.name = "Food",
		.desciption = "I recommend you take at least 200 pounds of food for each person in your family. I see that you have 5 people in all. You'll need flour, sugar, bacon, and coffee. My price is 20 cents a pound.",
		.image = "()O=-+\n    _\n   \\ \\",
		.items = (struct StoreItem[]) {
			{.name = "pounds of bacon", .price = .2f, .max = 1000},
			{.name = "pounds of flour", .price = .2f, .max = 1000}
		},
		.items_count = 2
	},
	{
		.name = "Clothing",
		.desciption = "You'll need warm clothing in the mountains. I recommend taking at least 2 sets of clothes per person, $10.00 each.",
		.image = "__\n||",
		.items = (struct StoreItem[]) {
			{.name = "sets of clothes", .price = 10.f, .max = 99}
		},
		.items_count = 1
	},
	{
		.name = "Ammunition",
		.desciption = "I sell ammunition in boxes of 20 bullets. Each box costs $2.00.",
		.image = "gun",
		.items = (struct StoreItem[]) {
			{.name = "boxes", .price = 2.f, .max = 99}
		},
		.items_count = 1
	},
	{
		.name = "Spare parts",
		.desciption = "It's a good idea to have a few spare parts for your wagon. Here are the prices:\n\n    axle - $10\n   wheel - $10\n  tongue - $10",
		.image = "O _",
		.items = (struct StoreItem[]) {
			{.name = "axles", .price = 10.f, .max = 3},
			{.name = "wheels", .price = 10.f, .max = 3},
			{.name = "tongues", .price = 10.f, .max = 3}
		},
		.items_count = 3
	}
};
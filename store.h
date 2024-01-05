#pragma once
#include "base.h"

void workStore(Coord input_pos);
Coord drawStore(void);

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
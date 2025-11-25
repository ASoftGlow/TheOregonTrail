#pragma once
#include "store.h"

struct StoreCategory STORE_MATT_CATEGORIES[] = {
	{
		.name = "Oxen",
		.desciption = "There are 2 oxen in a yoke; I recommend at least 3 yoke. I charge $40 a yoke.",
		.image = "o-uu,",
		.items = (struct StoreItem[]) {
			{.name = "yokes", .price = 40.f, .min = 1, .max = 9}
		},
		.items_count = 1,
	},
	{
		.name = "Food",
		.desciption = "I recommend you take at least 200 pounds of food for each person in your family. I see that you have 5 people in all. You'll need flour, sugar, bacon, and coffee. My price is 20 cents a pound.",
		.image = "()O=-+\n    _\n   \\ \\",
		.items = (struct StoreItem[]) {
			{.name = "pounds of bacon", .price = .2f, .max = 1000},
			{.name = "pounds of flour", .price = .2f, .max = 1000}
		},
		.items_count = 2,
	},
	{
		.name = "Clothing",
		.desciption = "You'll need warm clothing in the mountains. I recommend taking at least 2 sets of clothes per person, $10.00 each.",
		.image = "__\n||",
		.items = (struct StoreItem[]) {
			{.name = "sets of clothes", .price = 10.f, .max = 99}
		},
		.items_count = 1,
	},
	{
		.name = "Ammunition",
		.desciption = "I sell ammunition in boxes of 20 bullets. Each box costs $2.00.",
		.image = "gun",
		.items = (struct StoreItem[]) {
			{.name = "boxes", .price = 2.f, .max = 99}
		},
		.items_count = 1,
	},
	{
		.name = "Spare parts",
		.desciption = "It's a good idea to have a few spare parts for your wagon. Here are the prices:\n\n    axle - $10\n   wheel - $10\n  tongue - $10",
		.image = "O _",
		.items = (struct StoreItem[]) {
			{.name = "axles", .price = 10.f, .max = 3},
			{.name = "wheels", .price = 10.f, .max = 3},
			{.name = "tongues", .price = 10.f, .max = 3},
		},
		.items_count = 3,
	}
};

#pragma once
#include <stdbool.h>

#include "ansi_codes.h"

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
  const int items_count;
  float spent;
};

struct Store
{
  const char name[32];
  const enum Color color, color_category;
  struct StoreCategory* categories;
  const int categories_count;

  bool (*const callback_leave)(const struct Store* store);
};

void showStore(struct Store* store);
void showStoreAlert(char* text);

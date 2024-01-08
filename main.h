#pragma once
#include "base.h"
#include "static.h"

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

enum Injury
{
	INJURY_NONE,
	INJURY_BROKEN_ARM,
	INJURY_BROKEN_LEG
};

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

enum Weather
{
	WEATHER_MILD,
	WEATHER_COLD,
	WEATHER_HOT,
	WEATHER_RAIN,
	WEATHER_SNOW
};

enum Pace
{
	PACE_STEADY,
	PACE_2
};

enum Ration
{
	RATION_FILLING
};

void showMainMenu(void);
void showMonth(void);
void showRole(void);
void showStore(void);
#pragma once
#include "static.h"

#include <stdio.h>

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

typedef short MapMarkDensity;

#define	MAP_MARK_DENSITY_NORMAL 0
#define MAP_MARK_DENSITY_DOUBLE 1
#define MAP_MARK_DENSITY_SMUDGED 2

struct MapMark
{
	byte pos, path_index;
	MapMarkDensity density;
};

void saveState(const char* path);
void loadState(const char* path);

struct State
{
	float money;
	int bullets,
		clothing_sets,
		oxen,
		wagon_axles,
		wagon_wheels,
		wagon_torques;

	byte water;
	short food;

	byte month, day;
	enum Weather weather;
	enum Pace pace;
	enum Ration ration;
	enum Role role;

	char location[32];
	unsigned short progress;
	struct MapMark map_marks[32];
	byte map_marks_count;
	bool map_viewed;

	struct WagonMember wagon_leader[NAME_SIZE + 1];
	struct WagonMember wagon_members[WAGON_MEMBER_COUNT];
};

struct Settings
{
	bool no_tutorials;
	char save_path[FILENAME_MAX];
};


extern struct State state;
extern struct Settings settings;
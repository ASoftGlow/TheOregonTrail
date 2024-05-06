#pragma once
#include <stdio.h>

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

typedef short MapMarkDensity;

#define	MAP_MARK_DENSITY_NORMAL 0
#define MAP_MARK_DENSITY_DOUBLE 1
#define MAP_MARK_DENSITY_SMUDGED 2

struct MapMark
{
	byte pos, path_index;
	MapMarkDensity density;
};

enum StateStage
{
	STATE_STAGE_TUTORIAL = -1,
	STATE_STAGE_NONE,
	STATE_STAGE_START
};

bool saveState(const char* path);
bool loadState(const char* path);
bool saveSettings(void);
bool loadSettings(void);
void updateScreenSize(void);
void updateAutoScreenSize(void);

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
	// Use `setActivity`!
	char activity[32];
	unsigned short progress;
	struct MapMark map_marks[32];
	byte map_marks_count;
	bool map_viewed;

	struct WagonMember wagon_leader[NAME_SIZE + 1];
	struct WagonMember wagon_members[WAGON_MEMBER_COUNT];

	enum StateStage stage;
};

struct Settings
{
	bool no_tutorials;
	bool auto_save;
	char auto_save_path[FILENAME_MAX];
	bool auto_screen_size;
	int screen_width;
	int screen_height;
	bool discord_rp;
};

#define MIN_SCREEN_WIDTH 32
#define MAX_SCREEN_WIDTH 100
#define MIN_SCREEN_HEIGHT 16
#define MAX_SCREEN_HEIGHT 60
#define DEBUG_SAVE_PATH "../../resources/save.dat"

extern struct State state;
extern struct Settings settings;

void setActivity(const char*);
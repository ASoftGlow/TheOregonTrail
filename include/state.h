#pragma once
#include <stdint.h>

#include "static.h"

#define FORCE_ENUM

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
  PACE_STRENUOUS,
  PACE_GRUELING
};

enum Ration
{
  RATION_FILLING,
  RATION_MEAGER,
  RATION_BARE_BONES
};

typedef uint16_t MapMarkDensity;

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

const char* state_save(const char* path);
const char* state_load(const char* path);
const char* getSettingsPath(void);
int settings_save(void);
int settings_load(void);
void settings_free(void);
void updateScreenSize(void);
void updateAutoScreenSize(void);
void updateDiscordSupport(void);
void updateVolume(void);

struct State
{
  float money;
  uint8_t bullets, clothing_sets, oxen, wagon_axles, wagon_wheels, wagon_tongues;

  byte water;
  uint16_t food;

  byte month, day;
  enum Weather weather : 8;
  enum Pace pace : 8;
  enum Ration ration : 8;
  enum Role role : 8;

  uint16_t progress;
  struct MapMark map_marks[32];
  byte map_marks_count;
  bool map_viewed;

  struct WagonMember wagon_leader;
  struct WagonMember wagon_members[WAGON_MEMBER_COUNT];

  enum StateStage stage : 8;

  // temp below
  char location[32];
  // Use `setActivity`!
  char activity[32];
};

struct Settings
{
  bool no_tutorials;
  bool discord_rp;
  bool auto_save;
  bool auto_screen_size;
  char* auto_save_path;
  int32_t screen_width;
  int32_t screen_height;
  uint32_t volume;
};

#define SETTINGS_VERSION  'A'
#define SAVE_VERSION      'A'
#define MIN_SCREEN_WIDTH  32
#define MAX_SCREEN_WIDTH  248
#define MIN_SCREEN_HEIGHT 20
#define MAX_SCREEN_HEIGHT 93
#define DEBUG_SAVE_PATH   "../resources/save.dat"

extern struct State state;
extern struct Settings settings;

void setActivity(const char*);
void refreshActivity(void);
void state_autoSave(void);

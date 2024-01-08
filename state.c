#include "state.h"

float money = 40.f;
int
bullets = 0,
clothing_sets = 0,
oxen = 0,
wagon_axles = 0,
wagon_wheels = 0,
wagon_torques = 0;
byte water = 255;
short food;
enum Role role;
enum Weather weather;
byte month;
byte day = 1;
struct WagonMember wagon_leader[NAME_SIZE + 1];
struct WagonMember wagon_members[WAGON_MEMBER_COUNT];
char location[32];
enum Pace pace = PACE_STEADY;
enum Ration ration = RATION_FILLING;
#pragma once
#include "main.h"

extern float money;
extern int
bullets,
clothing_sets,
oxen,
wagon_axles,
wagon_wheels,
wagon_torques;
extern byte water;
extern short food;
extern enum Role role;
extern byte month;
extern byte day;
extern enum Weather weather;
extern struct WagonMember wagon_leader[NAME_SIZE + 1];
extern struct WagonMember wagon_members[WAGON_MEMBER_COUNT];
extern char location[32];
extern enum Pace pace;
extern enum Ration ration;
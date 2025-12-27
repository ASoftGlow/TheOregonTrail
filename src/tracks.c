#include "music_track_setup.h"

struct Track track0 = { .tempo = 0.5, .channels = {
  Channel(NONE, {_}),  
  Channel(TRIANGLE, {1, 0, 0.3}, {_}),
  //Channel(SQUARE,  LOOP(3), S(0.2, 0.5, 0, 0.5), LOOP(1), S(0.4, 0.2, 3, 0.3), LOOP(), {1}),
  Channel(NONE, {_}),
  Channel(NONE, {_}),
} };

// Channel(SAWTOOTH, {0.0, 2, 0.5}, {0.5, 0, _}, {0.5, -2, _}, {0.5, _, 0.0}, {0.5}),

#include "music_track_setdown.h"

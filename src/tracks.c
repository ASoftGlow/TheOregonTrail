#include "music.h"

#define Channel(waveform, events...)                        \
  { WAVEFORM_##waveform, (struct ChannelEvent[]){ events }, \
    sizeof((struct ChannelEvent[]){ events }) / sizeof(struct ChannelEvent) }
#define S(offset, duration, pitch, volume) { offset, pitch, volume }, { duration, _, 0 }
#define _                                  MUSIC_SPECIAL_VAL

//

struct Track track0 = { .tempo = .5, .channels = {
  Channel(SAWTOOTH, {0.0, 2, 0.5}, {0.5, 0, _}, {0.5, -2, _}, {0.5, _, 0.0}, {0.5}),
  Channel(TRIANGLE, {0.0, 0, 0.6}, {1.5, 0, 0}),
  Channel(NONE),
  Channel(NOISE,    S(0.0, 0.05, _, 0.5), S(0.2, 0.05, _, 0.5), S(0.2, 0.05, _, 0.5), S(0.2, 0.05, _, 0.5)),

  /*Channel(SQUARE,   {0.0, 0, 0.5}, {0.8, 2, 0.5}, {0.8}),
  Channel(TRIANGLE, {2.0, 0, 0.5}, {0.8, 2, 0.5}, {0.8}),
  Channel(SAWTOOTH, {4.0, 0, 0.5}, {0.8, 2, 0.5}, {0.8}),
  Channel(NOISE,    {6.0, 0, 0.5}, {0.8, 2, 0.5}, {0.8}),*/
} };

//

#undef Channel
#undef S
#undef _

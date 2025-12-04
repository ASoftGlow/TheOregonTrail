#include "music.h"

#define Channel(waveform, events...)                        \
  { WAVEFORM_##waveform, (struct ChannelEvent[]){ events }, \
    sizeof((struct ChannelEvent[]){ events }) / sizeof(struct ChannelEvent) }
#define S(offset, duration, pitch, volume) { offset, pitch, volume }, { offset + duration, _, 0 }
#define _                                  MUSIC_SPECIAL_VAL

//

struct Track track0 = { .tempo = 0.5, .channels = {
  Channel(SAWTOOTH, {0.0, 2, 0.5}, {0.5, 0, _}, {0.5, -2, _}, {0.5, _, 0.0}, {1.0}),
  Channel(NONE),
} };

//

#undef Channel
#undef S
#undef _

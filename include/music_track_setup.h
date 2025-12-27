#include "music.h"

#define Channel(waveform, ...)                                   \
  { WAVEFORM_##waveform, (struct ChannelEvent[]){ __VA_ARGS__ }, \
    sizeof((struct ChannelEvent[]){ __VA_ARGS__ }) / sizeof(struct ChannelEvent) }
#define S(offset, duration, pitch, volume) { offset, pitch, volume }, { duration, _, 0 }
#define _                                  MUSIC_SPECIAL_KEEP
#define LOOP(x)                            { MUSIC_SPECIAL_LOOP, x }

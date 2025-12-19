#pragma once
#include <stdbool.h>

#define MUSIC_NUM_CHANNELS 4
#define MUSIC_SPECIAL_KEEP -1.17549435e-38f // FLT_MIN
#define MUSIC_SPECIAL_LOOP -1.17549425e-38f

struct Channel
{
  enum WaveformType
  {
    WAVEFORM_NONE,
    WAVEFORM_SAWTOOTH,
    WAVEFORM_TRIANGLE,
    WAVEFORM_SQUARE,
    WAVEFORM_NOISE,
  } waveform_type;

  struct ChannelEvent
  {
    float offset;
    float freq;
    float volume;
  }* events;

  unsigned num_events;
};

struct Track
{
  bool processed;
  float tempo;
  struct Channel channels[MUSIC_NUM_CHANNELS];
};

bool music_setup(void);
void music_setdown(void);
bool music_play(struct Track* track, bool loop);
void music_setVolume(float volume);

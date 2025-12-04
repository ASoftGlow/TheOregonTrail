#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifndef _WIN32
#include <fcntl.h>
#endif

#include "base.h"
#include "music.h"
#include "portaudio/include/portaudio.h"

#define MUSIC_SAMPLE_RATE 44100

typedef struct
{
  float volume;
  bool loop;
  const struct Track* track;

  float time;

  struct ChannelState
  {
    unsigned i;
    float time;
    float value;
    int edge;
    float freq;
    float volume;
  } state[MUSIC_NUM_CHANNELS];
} MusicData;

static inline float
pitchToFreq(float pitch)
{
  return pow(2, ((double)pitch) / 12) * 440;
}

/*
 * called by PortAudio when audio frames are needed
 * may be in a sensitive context, so don't do IO
 */
static int
musicCallback(
    const void* input_buffer, void* output_buffer, unsigned long num_frames, const PaStreamCallbackTimeInfo* time_info,
    PaStreamCallbackFlags status_flags, void* user_data
)
{
  PaStreamCallbackResult result = paContinue;
  MusicData* data = (MusicData*)user_data;
  float* out = (float*)output_buffer;

  for (unsigned i = 0; i < num_frames; i++)
  {
    data->time += data->track->tempo / MUSIC_SAMPLE_RATE;
    unsigned finished_channels = 0;
    float value = 0.0f;
    for (unsigned c = 0; c < MUSIC_NUM_CHANNELS; c++)
    {
#define state   data->state[c]
#define channel data->track->channels[c]

      if (state.i == channel.num_events)
      {
        if (!data->loop)
        {
          if (++finished_channels == MUSIC_NUM_CHANNELS) result = paComplete;
          continue;
        }
        if (state.i == 0) continue;
        state.i = 0;
      }

      float volume = state.volume * data->volume;
      switch (channel.waveform_type)
      {
      case WAVEFORM_NONE: abort(); // should never get here

      case WAVEFORM_SAWTOOTH:
        state.value += volume * state.freq / MUSIC_SAMPLE_RATE;
        if (state.value >= volume) state.value = -volume;
        break;

      case WAVEFORM_TRIANGLE:
        state.value += volume * state.freq * 2 / MUSIC_SAMPLE_RATE * state.edge;
        if (state.value >= volume)
        {
          state.value = volume;
          state.edge = -1;
        }
        else if (state.value <= -volume)
        {
          state.value = -volume;
          state.edge = 1;
        }
        break;

      case WAVEFORM_NOISE:
        // TODO
        break;
      }
      value += state.value;

      const struct ChannelEvent event = channel.events[state.i];
      if (state.time + event.offset <= data->time)
      {
        state.time += event.offset;
        state.value = 0;
        state.edge = 1;
        if (event.pitch != MUSIC_SPECIAL_VAL) state.freq = pitchToFreq(event.pitch);
        if (event.volume != MUSIC_SPECIAL_VAL) state.volume = event.volume;
        state.i++;
      }

#undef state
#undef channel
    }

    *out++ = value;
  }
  return result;
}

void
preprocessTrack(struct Track* track)
{
  float end = 0.0f;
  for (unsigned c = 0; c < MUSIC_NUM_CHANNELS; c++)
  {
    if (track->channels[c].waveform_type == WAVEFORM_NONE)
    {
      track->channels[c].num_events = 0;
      continue;
    }

    float channel_end = 0.0f;
    for (unsigned i = 0; i < track->channels[c].num_events; i++)
    {
      float offset = track->channels[c].events[i].offset;
      if (offset == MUSIC_SPECIAL_VAL)
      {
        if (i == track->channels[c].num_events - 1) continue;
        // erroneous special value
        abort();
      }
      channel_end += offset;
    }
    if (channel_end > end) end = channel_end;
  }

  for (unsigned c = 0; c < MUSIC_NUM_CHANNELS; c++)
  {
    if (track->channels[c].num_events)
    {
      float* offset = &track->channels[c].events[track->channels[c].num_events - 1].offset;
      if (*offset == MUSIC_SPECIAL_VAL) *offset = end;
    }
  }
}

static MusicData data;
static PaStream* stream = NULL;

bool
music_setup(void)
{
#ifdef _WIN32
  PaError err = Pa_Initialize();
#else
  // ignore audio APIs' errors caused by PortAudio while finding a suitable API
  bool redirected = !freopen("/dev/null", "w", stderr);
  PaError err = Pa_Initialize();
  if (redirected) (void)!freopen("/dev/tty", "w", stderr);
#endif

  if (err) goto error;

  err = Pa_OpenDefaultStream(
      &stream, 0, // no input channels
      1,          // mono output
      paFloat32,  // output format
      MUSIC_SAMPLE_RATE, paFramesPerBufferUnspecified, musicCallback, &data
  );
  if (err) goto error;

  return 0;
error:
  EXIT_MSG = Pa_GetErrorText(err);
  return err;
}

void
music_setdown(void)
{
  if (stream) Pa_CloseStream(stream);
  Pa_Terminate();
}

bool
music_play(struct Track* track, bool loop)
{
  if (!track || Pa_IsStreamActive(stream))
  {
    Pa_StopStream(stream);
  }

  if (!track->processed)
  {
    preprocessTrack(track);
    track->processed = true;
  }

  memset(&data.state, 0, sizeof(data.state));
  data.time = 0.0f;
  data.track = track;
  data.loop = loop;
  return Pa_StartStream(stream);
}

void
music_setVolume(float volume)
{
  data.volume = volume;
}

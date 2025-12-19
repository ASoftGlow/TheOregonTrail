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

  struct ChannelState
  {
    unsigned i;
    float last_time, time;
    float value;
    int edge;
    float freq;
    float volume;
    float edge_time;

    struct
    {
      unsigned start_i, x;
      float start_time;
    } loop;
  } states[MUSIC_NUM_CHANNELS];
} MusicData;

static inline float
pitchToFreq(float pitch)
{
  return pow(2, ((double)pitch) / 12) * 440;
}

static void
resetTimeline(MusicData* data)
{
  memset(data->states, 0, sizeof(data->states));
  for (unsigned c = 0; c < MUSIC_NUM_CHANNELS; c++)
  {
    /*if (data->track->channels[c].num_events)
    {
      data->states[c].target_time = data->track->channels[c].events[0].offset;
    }*/
  }
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
  (void)input_buffer;
  (void)time_info;
  (void)status_flags;

  PaStreamCallbackResult result = paContinue;
  MusicData* data = (MusicData*)user_data;
  float* out = (float*)output_buffer;

  for (unsigned i = 0; i < num_frames; i++)
  {
    unsigned finished_channels = 0;
    float value = 0.0f;
    for (unsigned c = 0; c < MUSIC_NUM_CHANNELS; c++)
    {
#define state   data->states[c]
#define channel data->track->channels[c]

      // timeline control logic

      // check for end
      if (state.i == channel.num_events)
      {
        if (++finished_channels == MUSIC_NUM_CHANNELS)
        {
          if (data->loop)
          {
            resetTimeline(data);
          }
          else
          {
            result = paComplete;
          }
          // the last channel, so the continue acts as a break
        }
        continue;
      }

      // loops / next event
      struct ChannelEvent event = channel.events[state.i];
      // last_time <= time, so (offset <= 0) will cause it to
      // trigger instantly, like MUSIC_SPECIAL_LOOP
      if (state.last_time + event.offset <= state.time)
      {
        if (event.offset == MUSIC_SPECIAL_LOOP)
        {
          if (state.loop.x)
          {
            // existing loop
            state.loop.x--;
            state.i = state.loop.start_i;
            state.time = state.loop.start_time;
            state.last_time = state.loop.start_time;
          }
          else
          {
            // new loop
            state.loop.start_i = state.i + 1;
            state.loop.start_time = state.last_time + event.offset;
            state.loop.x = event.freq; // times
            state.i++;
            c--; // process channel again, next event
            continue;
          }
        }
        else
        {
          state.i++;
        }

        state.value = 0;
        state.edge = 1;
        state.edge_time = 0;
        state.last_time += event.offset;
        state.freq = event.freq;
        state.volume = event.volume;
      }

      // waveform generation

      float volume = state.volume * data->volume;
      switch (channel.waveform_type)
      {
      case WAVEFORM_NONE: break; // should never get here

      case WAVEFORM_SAWTOOTH:
        state.value += volume * state.freq / MUSIC_SAMPLE_RATE;
        if (state.value >= volume) state.value = -volume;
        break;

      case WAVEFORM_TRIANGLE:
        // the quietest, so boost it
#define TRI_MUL 2
        state.value += volume * TRI_MUL * state.freq * 2 / MUSIC_SAMPLE_RATE * state.edge;
        if (fabs(state.value) >= volume * TRI_MUL)
        {
          state.value = volume * TRI_MUL * state.edge;
          state.edge *= -1;
        }
        break;

      case WAVEFORM_SQUARE:
        if (state.edge_time <= state.time)
        {
          state.edge_time = state.time + 1 / state.freq * data->track->tempo;
          state.edge *= -1;
          state.value = volume * state.edge;
        }
        break;

      case WAVEFORM_NOISE:
        state.value = volume * (rand() / (float)RAND_MAX * 2 - 1); // white noise
        break;
      }
      value += state.value;
      state.time += data->track->tempo / MUSIC_SAMPLE_RATE;

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

    float last_freq = 0.0f, last_volume = 0.0f;
    float loop_times = 0.0f;
    float loop_start = 0.0f;
    float channel_end = 0.0f;
    for (unsigned i = 0; i < track->channels[c].num_events; i++)
    {
      struct ChannelEvent* event = &track->channels[c].events[i];
      if (event->offset == MUSIC_SPECIAL_KEEP)
      {
        if (i == track->channels[c].num_events - 1) continue;
        // erroneous special value
        abort();
      }
      if (event->offset == MUSIC_SPECIAL_LOOP)
      {
        if (loop_times)
        {
          channel_end += (channel_end - loop_start) * (int)loop_times;
        }
        float frac = modff(event->freq, &loop_times);
        assert(frac == 0.0f);

        loop_start = channel_end;
        continue;
      }

      event->freq = pitchToFreq(event->freq);

      if (event->freq == MUSIC_SPECIAL_KEEP) event->freq = last_freq;
      else last_freq = event->freq;

      if (event->volume == MUSIC_SPECIAL_KEEP) event->volume = last_volume;
      else last_volume = event->volume;

      channel_end += event->offset;
    }
    if (channel_end > end) end = channel_end;
  }

  for (unsigned c = 0; c < MUSIC_NUM_CHANNELS; c++)
  {
    if (track->channels[c].num_events)
    {
      float* offset = &track->channels[c].events[track->channels[c].num_events - 1].offset;
      if (*offset == MUSIC_SPECIAL_KEEP) *offset = end;
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

  data.track = track;
  data.loop = loop;
  resetTimeline(&data);
  return Pa_StartStream(stream);
}

void
music_setVolume(float volume)
{
  data.volume = volume;
}

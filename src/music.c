#include <string.h>
#ifndef _WIN32
#include <fcntl.h>
#endif

#include "base.h"
#include "libsndfile/include/sndfile.h"
#include "music.h"
#include "portaudio/include/portaudio.h"

typedef struct
{
  float left_phase;
  float right_phase;
} paTestData;

static float bump = 0;

/* This routine will be called by the PortAudio engine when audio is needed.
 * It may called at interrupt level on some machines so don't do anything
 * that could mess up the system like calling malloc() or free().
 */
static int
patestCallback(
    const void* inputBuffer, void* outputBuffer, unsigned long framesPerBuffer, const PaStreamCallbackTimeInfo* timeInfo,
    PaStreamCallbackFlags statusFlags, void* userData
)
{
  /* Cast data passed through stream to our structure. */
  paTestData* data = (paTestData*)userData;
  float* out = (float*)outputBuffer;
  unsigned int i;
  (void)inputBuffer; /* Prevent unused variable warning. */

  for (i = 0; i < framesPerBuffer; i++)
  {
    *out++ = data->left_phase;  /* left */
    *out++ = data->right_phase; /* right */
    /* Generate simple sawtooth phaser that ranges between -1.0 and 1.0. */
    data->left_phase += 0.001f + bump;
    /* When signal reaches top, drop back down. */
    if (data->left_phase >= 0.2f) data->left_phase -= 0.1f;
    /* higher pitch so we can distinguish left and right. */
    data->right_phase = data->left_phase;
  }
  bump += 0.0001f;
  if (bump > 0.1f) bump = -0.0005f;
  return 0;
}

static paTestData data;
static PaStream* stream;

bool
music_setup(void)
{
#ifdef _WIN32
  PaError err = Pa_Initialize();
#else
  // ignore audio APIs' errors caused by PortAudio while
  // finding a suitable API
  freopen("/dev/null", "w", stderr);
  PaError err = Pa_Initialize();
  freopen("/dev/tty", "w", stderr);
#endif

  if (err)
  {
    EXIT_MSG = Pa_GetErrorText(err);
    return 1;
  }

  return 0;
}

void
music_setdown(void)
{
  if (stream && Pa_IsStreamActive(stream)) Pa_AbortStream(stream);
  Pa_Terminate();
}

bool
music_play(const char* soundFile)
{
#define BUFFER_LEN 1024

  SNDFILE* sndFile = NULL;
  SF_INFO sfInfo;
  memset(&sfInfo, 0, sizeof(sfInfo));
  sndFile = sf_open(soundFile, SFM_READ, &sfInfo);
  stream = NULL;
  if (!sndFile)
  {
    EXIT_MSG = "Sound file not found";
    return 1;
  }
  puts("test3");
  PaStreamParameters paStreamParameters
      = { .device = Pa_GetDefaultOutputDevice(),
          .channelCount = sfInfo.channels,
          .sampleFormat = paFloat32,
          .suggestedLatency = Pa_GetDeviceInfo(paStreamParameters.device)->defaultLowOutputLatency,
          .hostApiSpecificStreamInfo = NULL };
  const PaHostApiInfo* pai = Pa_GetHostApiInfo(Pa_GetDeviceInfo(paStreamParameters.device)->hostApi);
  PaError paError = Pa_OpenStream(
      &stream, NULL, &paStreamParameters, sfInfo.samplerate, paFramesPerBufferUnspecified, paNoFlag, NULL, NULL
  );
  if (paError || !stream) goto error;
  if ((paError = Pa_StartStream(stream))) goto error;

  sf_count_t readCount = 0;
  float data[BUFFER_LEN];
  memset(data, 0, sizeof(data));
  while (((readCount = sf_read_float(sndFile, data, BUFFER_LEN))))
  {
    if ((paError = Pa_WriteStream(stream, data, readCount))) goto error;
    memset(data, 0, sizeof(data));
  }
  if ((paError = Pa_CloseStream(stream))) goto error;
  return 0;

error:
  EXIT_MSG = Pa_GetErrorText(paError);
  return 1;
}

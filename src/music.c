#include "music.h"
#include "portaudio/include/portaudio.h"
#include "libsndfile/include/sndfile.h"
#include <string.h>

typedef struct
{
	float left_phase;
	float right_phase;
}
paTestData;
static float bump = 0;
/* This routine will be called by the PortAudio engine when audio is needed.
 * It may called at interrupt level on some machines so don't do anything
 * that could mess up the system like calling malloc() or free().
*/
static int patestCallback(const void* inputBuffer, void* outputBuffer,
	unsigned long framesPerBuffer,
	const PaStreamCallbackTimeInfo* timeInfo,
	PaStreamCallbackFlags statusFlags,
	void* userData)
{
	/* Cast data passed through stream to our structure. */
	paTestData* data = (paTestData*)userData;
	float* out = (float*)outputBuffer;
	unsigned int i;
	(void)inputBuffer; /* Prevent unused variable warning. */

	for (i = 0; i < framesPerBuffer; i++)
	{
		*out++ = data->left_phase;  /* left */
		*out++ = data->right_phase;  /* right */
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


bool music_setup(void)
{
	// sound stuff
	PaError err = Pa_Initialize();
	if (err != paNoError) goto pa_error;

	return 0;
pa_error:
	EXIT_MSG = Pa_GetErrorText(err);
	return 1;
}

void music_setdown(void)
{
	if (stream) Pa_AbortStream(stream);
	Pa_Terminate();
}

//bool music_play(const char* path)
//{
//	SNDFILE* sndFile = NULL;
//	SF_INFO sfInfo = { 0 };
//	sndFile = sf_open(path, SFM_READ, &sfInfo);
//	stream = NULL;
//	if (!sndFile)
//	{
//		EXIT_MSG = sf_strerror(sndFile);
//		return 1;
//	}
//	const PaDeviceIndex device = Pa_GetDefaultOutputDevice();
//	const PaDeviceInfo* device_i = Pa_GetDeviceInfo(device);
//	printf("Using %s sound output device\n", device_i->name);
//	fflush(stdout);
//	PaStreamParameters paStreamParameters = {
//		.device = device,
//		.channelCount = sfInfo.channels,
//		.sampleFormat = paInt16,
//		.suggestedLatency = device_i->defaultLowOutputLatency,
//		.hostApiSpecificStreamInfo = NULL
//	};
//	PaError paError = Pa_OpenStream(&stream, NULL, &paStreamParameters, sfInfo.samplerate, paFramesPerBufferUnspecified, paClipOff, NULL, NULL);
//	if (paError || !stream) goto pa_error;
//	paError = Pa_StartStream(stream);
//	if (paError) goto pa_error;
//
//	/*int subFormat = sfInfo.format & SF_FORMAT_SUBMASK;
//	double scale = 1.0;
//	if (subFormat == SF_FORMAT_FLOAT || subFormat == SF_FORMAT_DOUBLE)
//	{
//		sf_command(sndFile, SFC_CALC_SIGNAL_MAX, &scale, sizeof(scale));
//		if (scale < 1e-10)
//		{
//			scale = 1.0;
//		}
//		else
//		{
//			scale = 32700.0 / scale;
//		}
//	}*/
//	sf_count_t readCount = 0;
//#define BUFFER_LEN 1024
//	float data[BUFFER_LEN] = { 0 };
//	while ((readCount = sf_read_float(sndFile, data, BUFFER_LEN)))
//	{
//		/*if (subFormat == SF_FORMAT_FLOAT || subFormat == SF_FORMAT_DOUBLE)
//		{
//			for (int m = 0; m < readCount; ++m)
//			{
//				data[m] *= scale;
//			}
//		}*/
//		for (int m = 0; m < readCount; ++m)
//		{
//			data[m] *= 10.0f;
//		}
//		paError = Pa_WriteStream(stream, data, BUFFER_LEN);
//		if (paError) goto pa_error;
//		memset(data, 0, sizeof(data));
//		puts("Wrote data");
//		fflush(stdout);
//	}
//	paError = Pa_CloseStream(stream);
//#undef BUFFER_LEN
//	return 0;
//pa_error:
//	EXIT_MSG = Pa_GetErrorText(paError);
//	if (stream)
//	{
//		Pa_AbortStream(stream);
//		stream = NULL;
//	}
//	return 1;
//}

bool music_play(const char* soundFile)
{
#define BUFFER_LEN 1024

	PaError paError = Pa_Initialize();
	if (paError != paNoError)
	{
		return 0;
	}
	SNDFILE* sndFile = NULL;
	SF_INFO sfInfo;
	memset(&sfInfo, 0, sizeof(sfInfo));
	sndFile = sf_open(soundFile, SFM_READ, &sfInfo);
	if (!sndFile) goto error;
	PaStream* stream = NULL;
	PaStreamParameters paStreamParameters = {
		.device = Pa_GetDefaultOutputDevice(),
		.channelCount = sfInfo.channels,
		.sampleFormat = paInt16,
		.suggestedLatency = Pa_GetDeviceInfo(paStreamParameters.device)->defaultLowOutputLatency,
		.hostApiSpecificStreamInfo = NULL
	};
	paError = Pa_OpenStream(
		&stream, NULL, &paStreamParameters,
		sfInfo.samplerate, paFramesPerBufferUnspecified, paClipOff,
		NULL, NULL);
	if (paError || !stream) goto error;
	paError = Pa_StartStream(stream);
	if (paError) goto error;
	int subFormat = sfInfo.format & SF_FORMAT_SUBMASK;
	double scale = 1.0;
	if (subFormat == SF_FORMAT_FLOAT || subFormat == SF_FORMAT_DOUBLE)
	{
		sf_command(sndFile, SFC_CALC_SIGNAL_MAX, &scale, sizeof(scale));
		if (scale < 1e-10)
		{
			scale = 1.0;
		}
		else
		{
			scale = 32700.0 / scale;
		}
	}
	sf_count_t readCount = 0;
	float data[BUFFER_LEN];
	memset(data, 0, sizeof(data));
	while ((readCount = sf_read_float(sndFile, data, BUFFER_LEN)))
	{
		if (subFormat == SF_FORMAT_FLOAT || subFormat == SF_FORMAT_DOUBLE)
		{
			for (int m = 0; m < readCount; ++m)
			{
				data[m] *= scale;
			}
		}
		paError = Pa_WriteStream(stream, data, BUFFER_LEN);
		if (paError) goto error;
		memset(data, 0, sizeof(data));
	}
	paError = Pa_CloseStream(stream);
	if (paError) goto error;
	Pa_Terminate();
	return 1;
error:
	EXIT_MSG = Pa_GetErrorText(paError);
	if (stream)
	{
		Pa_AbortStream(stream);
		stream = NULL;
	}
	Pa_Terminate();
	return 1;
}
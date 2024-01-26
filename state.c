#include <errno.h>

#include "state.h"
#include "main.h"

struct State state = {
	.water = -1,
	.day = 1,
	.location = "Missing"
};

struct Settings settings = { 0 };

bool saveSettings(void)
{
	errno = 0;
	FILE* f = fopen(SETTINGS_PATH, "wb");
	if (errno) return 0;
	fwrite(&settings, 1, sizeof(settings), f);
	fclose(f);
	return 1;
}

bool loadSettings(void)
{
	errno = 0;
	FILE* f = fopen(SETTINGS_PATH, "rb");
	if (errno) return 0;
	fread(&settings, 1, sizeof(settings), f);
	fclose(f);
	return 1;
}

bool saveState(const char* path)
{
	errno = 0;
	FILE* f = fopen(path, "wb");
	if (errno) return 0;
	fwrite(&state, 1, sizeof(state), f);
	fclose(f);
	return 1;
}

bool loadState(const char* path)
{
	errno = 0;
	FILE* f = fopen(path, "rb");
	if (errno) return 0;
	fread(&state, 1, sizeof(state), f);
	fclose(f);
	if (errno) return 0;

	switch (state.stage)
	{
	case STATE_STAGE_START:
		showMain();
		break;

	case STATE_STAGE_BEGIN:
	default:
		return 0;
		break;
	}
	return 1;
}
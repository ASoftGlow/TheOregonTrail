#include <errno.h>

#include "state.h"
#include "main.h"
#include "tui.h"
#include "map.h"
#include "input.h"

struct State state = {
	.water = -1,
	.day = 1,
	.location = "Missing"
};

struct Settings settings = {
	0,
	.auto_save_path = "save.dat",
	.screen_width = 40,
	.screen_height = 7
};

bool saveSettings(void)
{
	errno = 0;
	FILE* f = fopen(SETTINGS_PATH, "wb");
	if (errno) return 0;
	fwrite(&settings, 1, sizeof(settings), f);
	fclose(f);
	return 1;
}

static bool postSettings(bool ret)
{
	updateScreenSize();
	return ret;
}

bool loadSettings(void)
{
	errno = 0;
	FILE* f = fopen(SETTINGS_PATH, "rb");
	if (errno) return postSettings(0);
	fread(&settings, 1, sizeof(settings), f);
	fclose(f);

	return postSettings(1);
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

	default:
		return 0;
		break;
	}
	return 1;
}

void updateScreenSize()
{
	if (settings.auto_screen_size)
	{
		Coord size = getScreenSize();
		settings.screen_width = size.x;
		settings.screen_height = size.y;
	}

	if (settings.screen_width < MIN_SCREEN_WIDTH) settings.screen_width = MIN_SCREEN_WIDTH;
	if (settings.screen_width > MAX_SCREEN_WIDTH) settings.screen_width = MAX_SCREEN_WIDTH;
	if (settings.screen_height < MIN_SCREEN_HEIGHT) settings.screen_height = MIN_SCREEN_HEIGHT;
	if (settings.screen_height > MAX_SCREEN_HEIGHT) settings.screen_height = MAX_SCREEN_HEIGHT;

	SCREEN_WIDTH = settings.screen_width;
	SCREEN_HEIGHT = settings.screen_height;
	DIALOG_CONTENT_WIDTH = DIALOG_WIDTH - DIALOG_PADDING_X * 2;
	MAP_VIEWPORT_HEIGHT = SCREEN_HEIGHT - 2;
}
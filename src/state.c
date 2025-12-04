#include "cfgpath.h"
#include <string.h>

#include "input.h"
#include "main.h"
#include "map.h"
#include "state.h"
#include "tui.h"
#ifdef TOT_DISCORD
#include "discord.h"
#endif
#ifndef TOT_MUTE
#include "music.h"
#endif

char cfgfile[MAX_PATH] = { 0 };

struct State state = {
  .water = -1,
  .day = 1,
  .location = "Missing",
  .activity = "Loading",
};

struct Settings settings = {
  .version = SETTINGS_VERSION,
  .volume = 8,
  .auto_save_path = "save.dat",
  .screen_width = 40,
  .screen_height = 16,
  .discord_rp = 1,
};

const char*
getSettingsPath(void)
{
  return cfgfile;
}

int
saveSettings(void)
{
  FILE* f = fopen(cfgfile, "wb");
  if (!f) return 1;
  if (!fwrite(&settings, sizeof(settings), 1, f))
  {
    fclose(f);
    return 2;
  }
  fclose(f);
  return 0;
}

int
loadSettings(void)
{
  get_user_config_file(cfgfile, sizeof(cfgfile), "asoftglow-tot");
  if (!cfgfile[0]) return 9;

  FILE* f = fopen(cfgfile, "rb");
  if (!f) return 1;
  char version = 0;
  if (!fread(&version, 1, 1, f)) goto fail;
  switch (version)
  {
  case SETTINGS_VERSION:
    if (!fread((char*)&settings + sizeof(settings.version), sizeof(settings) - sizeof(settings.version), 1, f)) goto fail;
    break;

  default:
  fail:
    fclose(f);
    return 2;
  }
  fclose(f);
  return 0;
}

int
saveState(const char* path)
{
  FILE* f = fopen(path, "wb");
  if (!f) return 1;
  if (!fwrite(&state, sizeof(state), 1, f))
  {
    return 2;
    fclose(f);
  }
  fclose(f);
  return 0;
}

int
loadState(const char* path)
{
  FILE* f = fopen(path, "rb");
  if (!f) return 1;
  if (!fread(&state, sizeof(state), 1, f))
  {
    fclose(f);
    return 2;
  }
  fclose(f);
  setActivity(state.activity);

  switch (state.stage)
  {
  case STATE_STAGE_START: showMain(); break;

  default:                return 3;
  }
  return 0;
}

void
updateScreenSize()
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
  DIALOG_CONTENT_WIDTH = DIALOG_WIDTH - DIALOG_PADDING_X * 2 - 2;
  MAP_VIEWPORT_HEIGHT = SCREEN_HEIGHT - 2;

  setScreenSize(SCREEN_WIDTH, SCREEN_HEIGHT);
}

void
updateAutoScreenSize(void)
{
  updateScreenSize();
  if (settings.auto_screen_size) enableResizing();
  else disableResizing();
}

#ifdef TOT_DISCORD
void
updateDiscordSupport(void)
{
  if (settings.discord_rp)
  {
    discord_setup();
    refreshActivity();
  }
  else discord_setdown();
}
#endif

#ifndef TOT_MUTE
void
updateVolume(void)
{
  music_setVolume(settings.volume / 9.0f);
}
#endif

void
setActivity(const char* activity)
{
  if (strcmp(state.activity, activity))
  {
    strcpy(state.activity, activity);
    refreshActivity();
  }
}

void
refreshActivity(void)
{
#ifdef TOT_DISCORD
  if (settings.discord_rp) discord_update_activity(state.activity);
#endif
}

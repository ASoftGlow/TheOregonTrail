#include "cfgpath/cfgpath.h"
#include <stdio.h>
#include <string.h>

#include "input.h"
#include "state.h"
#include "tui.h"
#ifdef TOT_DISCORD
#include "discord.h"
#endif
#ifndef TOT_MUTE
#include "music.h"
#endif

static char cfgfile[MAX_PATH] = { 0 };

struct State state = {
  .water = -1,
  .day = 1,
  .location = "Missing",
  .activity = "Loading",
};
const size_t STATE_DISK_SIZE = &state.__disk_end - (byte*)&state;

const struct Settings DEFAULT_SETTINGS = {
  .volume = 8,
  .auto_save_path = "save.dat",
  .screen_width = 40,
  .screen_height = 16,
  .discord_rp = 1,
};

struct Settings settings = DEFAULT_SETTINGS;

const char*
getSettingsPath(void)
{
  return cfgfile;
}

#define write(v) fwrite(&v, sizeof(v), 1, f)

int
saveSettings(void)
{
  // clang-format off
  uint16_t flags = settings.no_tutorials     << 0
                 | settings.auto_save        << 1
                 | settings.auto_screen_size << 2
                 | settings.discord_rp       << 3;
  // clang-format on

  FILE* f = fopen(cfgfile, "wb");
  if (!f) return 1;

  if (fputc(SETTINGS_VERSION, f) < 0           //
      || !write(flags)                         //
      || !write(settings.screen_width)         //
      || !write(settings.screen_height)        //
      || !write(settings.volume)               //
      || fputs(settings.auto_save_path, f) < 0 //
  )
  {
    fclose(f);
    return 2;
  }

  fclose(f);
  return 0;
}

#undef write
#define read(v) fread(&v, sizeof(v), 1, f)

int
loadSettings(void)
{
  get_user_config_file(cfgfile, sizeof(cfgfile), "asoftglow-tot");
  if (!cfgfile[0]) strcpy(cfgfile, "asoftglow-tot.config");

  FILE* f = fopen(cfgfile, "rb");
  if (!f) return 1;
  switch (fgetc(f))
  {
  case 'A':
  {
    fseek(f, 0, SEEK_END);
    long path_length = ftell(f) - /* version */ 1 - sizeof(int16_t) - sizeof(int32_t) * 3;
    fseek(f, /* version */ 1, SEEK_SET);

    settings.auto_save_path = malloc(sizeof(struct Settings) + path_length + 1);
    if (!settings.auto_save_path) goto fail;

    uint16_t flags;
    if (!fread(&flags, sizeof(flags), 1, f) //
        || !read(settings.screen_width)     //
        || !read(settings.screen_height)    //
        || !read(settings.volume)           //

        || !fread(settings.auto_save_path, path_length, 1, f))
    {
      free(settings.auto_save_path);
      settings.auto_save_path = NULL;
      goto fail;
    }

    settings.auto_save_path[path_length] = '\0';

    // clang-format off
    settings.no_tutorials     = flags & (1 << 0);
    settings.auto_save        = flags & (1 << 1);
    settings.auto_screen_size = flags & (1 << 2);
    settings.discord_rp       = flags & (1 << 3);
    // clang-format on
    break;
  }

  default:
  fail:
    fclose(f);
    return 2;
  }
  fclose(f);
  return 0;
}

#undef read

const char*
saveState(const char* path)
{
  FILE* f = fopen(path, "wb");
  if (!f) return "open failed";
  if (fputc(SAVE_VERSION, f) < 0 //
      || !fwrite(&state, STATE_DISK_SIZE, 1, f))
  {
    return "write failed";
    fclose(f);
  }
  fclose(f);
  return NULL;
}

const char*
loadState(const char* path)
{
  FILE* f = fopen(path, "rb");
  if (!f) return "open failed";
  switch (fgetc(f))
  {
  case 'A':
    if (fread(&state, STATE_DISK_SIZE, 1, f))
    {
      break;
    }

  case EOF:
    fclose(f); //
    return "read failed";

  default:
    fclose(f); //
    return "unknown version";
  }

  fclose(f);

  switch (state.stage)
  {
  case STATE_STAGE_START: return NULL;

  default:                return "unknown stage";
  }
}

void
autoSave(void)
{
  if (settings.auto_save && *settings.auto_save_path) saveState(settings.auto_save_path);
}

byte MAP_VIEWPORT_HEIGHT;

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

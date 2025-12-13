#include "screens.h"
#include "settings.h"
#include "state.h"

static const struct Setting main_settings[] = {
#ifndef TOT_MUTE
  { .name = "Volume",
                  .p = { .fractional = &settings.volume },
                  .type = SETTING_TYPE_FRACTIONAL,
                  .callback = &updateVolume,
                  .max = 9,
                  .live = true },
#endif
  { .name = "Skip tutorials", .p = { .boolean = &settings.no_tutorials }, .type = SETTING_TYPE_BOOLEAN },
  { .name = "Auto save", .p = { .boolean = &settings.auto_save }, .type = SETTING_TYPE_BOOLEAN },
  { .name = "Auto save path", .p = { .dynamic_string = &settings.auto_save_path }, .type = SETTING_TYPE_PATH },
  { .name = "Auto screen size",
                  .p = { .boolean = &settings.auto_screen_size },
                  .type = SETTING_TYPE_BOOLEAN,
                  .callback = &updateAutoScreenSize },
  { .name = "Screen width",
                  .p = { .number = &settings.screen_width },
                  .type = SETTING_TYPE_NUMBER,
                  .callback = &updateScreenSize,
                  .min = 30 },
  { .name = "Screen height",
                  .p = { .number = &settings.screen_height },
                  .type = SETTING_TYPE_NUMBER,
                  .callback = &updateScreenSize,
                  .min = 10 },
#ifdef TOT_DISCORD
  { .name = "Enable Discord rich presence",
                  .p = { .boolean = &settings.discord_rp },
                  .type = SETTING_TYPE_BOOLEAN,
                  .callback = &updateDiscordSupport },
#endif
};

void
screen_mainSettings(void)
{
  showSettings(countof(main_settings), main_settings);
}

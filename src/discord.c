#include "discord.h"
#include "discord-rpc/include/discord_rpc.h"
#include <stdio.h>
#include <string.h>
#include <threads.h>

bool isSetup = 0;
DiscordRichPresence drp = {
  .largeImageKey = "tot",
  .largeImageText = "This is supposed to be a wagon",
};

bool
discord_setup(void)
{
  Discord_Initialize("1226738337625145446", NULL, 0, NULL);
  isSetup = 1;
  return 0;
}

bool
discord_update_activity(const char* details)
{
  drp.details = details;
  Discord_UpdatePresence(&drp);
  Discord_RunCallbacks();
  return 0;
}

void
discord_setdown(void)
{
  if (isSetup)
  {
    Discord_Shutdown();
    isSetup = 0;
  }
}

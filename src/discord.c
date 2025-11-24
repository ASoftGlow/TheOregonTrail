#include <stdio.h>
#include <threads.h>
#include <string.h>
#include "discord.h"
#include "discord-rpc/include/discord_rpc.h"

#include "input.h"

bool isSetup = 0;
char __details[128];
const DiscordRichPresence drp = {
	.largeImageKey = "tot",
	.largeImageText = "This is supposed to be a wagon",
	.details = __details
};

bool discord_setup(void)
{
	Discord_Initialize("1226738337625145446", NULL, 0, NULL);
	isSetup = 1;
	return 0;
}

bool discord_update_activity(const char* details)
{
	strcpy(__details, details);
	Discord_UpdatePresence(&drp);
	Discord_RunCallbacks();
	return 0;
}

void discord_setdown(void)
{
	if (isSetup)
	{
		Discord_ClearPresence();
		Discord_RunCallbacks();
		Discord_Shutdown();
		isSetup = 0;
	}
}
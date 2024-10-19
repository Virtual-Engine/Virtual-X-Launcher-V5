#include "Backend/Register.hpp"
#include"Backend/rpc.h"
#include <chrono>
#include "Backend/Handler.hpp"
#pragma comment(lib, "discord-rpc.lib")

namespace VX {
	DiscordHandler g_DiscordHandler;

	bool DiscordHandler::initialize() {
		DiscordEventHandlers handlers;
		std::memset(&handlers, 0, sizeof(handlers));
		Discord_Initialize("1201972275491115018", &handlers, 1, "0");
		return true;
	}

	void DiscordHandler::update() {


		DiscordRichPresence discordPresence;
		std::memset(&discordPresence, 0, sizeof(discordPresence));

		discordPresence.largeImageKey = "";
		discordPresence.state = "";
		discordPresence.instance = 1;
		Discord_UpdatePresence(&discordPresence);
	}

	void DiscordHandler::cleanup() {
		Discord_ClearPresence();
		Discord_Shutdown();
	}
	DiscordHandler* get_discord() { return &g_DiscordHandler; }
}
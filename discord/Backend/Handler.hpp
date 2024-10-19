#pragma once
#include "Register.hpp"
#include "rpc.h"

namespace VX {
    class DiscordHandler {
    public:
        bool initialize();
        static void update();
        void cleanup();
    };

    DiscordHandler* get_discord();
    inline DiscordRichPresence discordPresence;
}

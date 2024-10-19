#pragma once
#include <Windows.h>

#include "../discord/backend/handler.hpp"

#include "../fiber/fiber.hpp"
#include "../fiber/manager.hpp"

#include "../services/threads/threads_pool.hpp"

namespace VX {

	class core {
	public:
		void initialize();
		void cleanup();
	};
	core* get_core();
};
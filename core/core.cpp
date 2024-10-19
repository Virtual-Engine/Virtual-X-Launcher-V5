#include "core.hpp"
#include "../common.hpp"
#include "../backend/backend.hpp"

namespace VX {

	core g_core;
	void core::initialize() {
		g_logger = std::make_unique<OwnLogger>(NAME, false, true, true, false, "Launcher.log");
		g_logger->init("C:\\Zenith\\", 225, true, false);

		get_thread_pool()->initiliaze();
		get_renderer()->initialize();

		get_fiber_manager()->add_fiber(new fiber("API", &backend::api));
		get_fiber_manager()->add_fiber(new fiber("UI", &backend::renderer));
		get_fiber_manager()->add_fiber(new fiber("Injection Find", &backend::injection_find));
		get_fiber_manager()->add_fiber(new fiber("Discord", &backend::discord));

		LOG(INFO) << "Virtual X Launcher Loaded";
	}

	void core::cleanup() {
		std::this_thread::sleep_for(500ms);
		get_discord()->cleanup();
		get_fiber_manager()->cleanup();
		get_thread_pool()->cleanup();
		get_renderer()->cleanup();
		g_logger->destroy();
		g_logger.reset();
	}

	core* get_core() { return &g_core; };
}

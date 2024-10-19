#include "common.hpp"
#include "core/core.hpp"

using namespace VX;

int entrypoint() {
	get_core()->initialize();
	while (g_running) {
		get_fiber_manager()->initialize();
		while (PeekMessageA(&g_Msg, NULL, 0U, 0U, PM_REMOVE)) {
			TranslateMessage(&g_Msg);
			DispatchMessageA(&g_Msg);
			if (g_Msg.message == WM_QUIT)
				g_running = false;
		}
	}
	get_core()->cleanup();
}

int main() {
	return entrypoint();
}
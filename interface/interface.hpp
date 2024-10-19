#pragma once
#include "../common.hpp"
#include "../services/injection/injection.hpp"
#include "element.hpp"
#include "spinner.hpp"
#include "../renderer/renderer.hpp"

namespace VX {
	class ui
	{

	public:
		void update();
		void render();
		void auth();
		void injection();
		void injection_loading();

	private:
		bool auth_tab = true;
		bool injection_tab = false;
		bool injection_loading_tab = false;
	};

    ui* get_interface();
}
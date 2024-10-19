#include "backend.hpp"

namespace VX {
	void backend::api()
	{
		while (g_running)
		{
			fiber::current()->sleep();
		}
	}
	void backend::renderer()
	{
		while (g_running)
		{
			get_renderer()->present();

			fiber::current()->sleep();
		}
	}
	void backend::injection_find()
	{
		while (g_running) {

			if (!get_injection()->m_no_fail && get_injection()->is_opened()) {
				get_injection()->m_no_fail = true;
			}

			fiber::current()->sleep();
		}
	}
	void backend::discord()
	{
		get_discord()->initialize();
		while (g_running)
		{
			get_discord()->update();

			fiber::current()->sleep();
		}
	}
}
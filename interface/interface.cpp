#include "Interface.hpp"
#include <shellapi.h>

namespace VX {
	ui g_ui;

	void ui::update()
	{
		auto& io = ImGui::GetIO(); (void)io;
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

		auto&& style = ImGui::GetStyle();
		auto&& colors = style.Colors;

		style.Alpha = 1.f;
		style.WindowTitleAlign = ImVec2(0.f, 0.5f);
		style.WindowPadding = ImVec2(10, 5);
		style.WindowRounding = 5.f;
		style.FramePadding = ImVec2(8, 5);
		style.FrameRounding = 4.f;
		style.ItemSpacing = ImVec2(10, 10);
		style.ItemInnerSpacing = ImVec2(4, 5);
		style.IndentSpacing = 27.f;
		style.ScrollbarSize = 9.f;
		style.ScrollbarRounding = 12.f;
		style.GrabMinSize = 7.f;
		style.GrabRounding = 3.f;
		style.TouchExtraPadding = ImVec2();
		colors[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
		colors[ImGuiCol_TextDisabled] = ImVec4(0.33f, 0.33f, 0.34f, 1.00f);
		colors[ImGuiCol_WindowBg] = ImColor(0, 0, 0, 0);
		colors[ImGuiCol_ChildBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
		colors[ImGuiCol_PopupBg] = ImVec4(0.11f, 0.11f, 0.11f, 1.00f);
		//colors[ImGuiCol_Border] = ImColor(107, 5, 255, 170);
		colors[ImGuiCol_BorderShadow] = ImColor(107, 5, 255, 255);
		colors[ImGuiCol_FrameBg] = ImVec4(0.11f, 0.11f, 0.10f, 1.00f);
		colors[ImGuiCol_FrameBgHovered] = ImVec4(0.16f, 0.16f, 0.16f, 1.00f);
		colors[ImGuiCol_FrameBgActive] = ImVec4(0.04f, 0.04f, 0.04f, 1.00f);
		colors[ImGuiCol_TitleBg] = ImVec4(0.15f, 0.15f, 0.15f, 1.00f);
		colors[ImGuiCol_TitleBgActive] = ImVec4(0.15f, 0.15f, 0.15f, 1.00f);
		colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.15f, 0.15f, 0.15f, 0.75f);
		colors[ImGuiCol_MenuBarBg] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
		colors[ImGuiCol_ScrollbarBg] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
		colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.80f, 0.80f, 0.83f, 0.31f);
		colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
		colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
		colors[ImGuiCol_CheckMark] = ImColor(107, 5, 255, 170);
		colors[ImGuiCol_SliderGrab] = ImVec4(0.80f, 0.80f, 0.83f, 0.31f);
		colors[ImGuiCol_SliderGrabActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
		colors[ImGuiCol_Button] = ImVec4(0.16f, 0.16f, 0.16f, 1.00f);
		colors[ImGuiCol_ButtonHovered] = ImVec4(0.27f, 0.27f, 0.28f, 1.00f);
		colors[ImGuiCol_ButtonActive] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
		colors[ImGuiCol_Header] = ImVec4(0.10f, 0.10f, 0.11f, 1.00f);
		colors[ImGuiCol_HeaderHovered] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
		colors[ImGuiCol_HeaderActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
		colors[ImGuiCol_Separator] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
		colors[ImGuiCol_SeparatorHovered] = ImVec4(0.24f, 0.23f, 0.29f, 1.00f);
		colors[ImGuiCol_SeparatorActive] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
		colors[ImGuiCol_ResizeGrip] = ImVec4(1.00f, 1.00f, 1.00f, 0.00f);
		colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
		colors[ImGuiCol_ResizeGripActive] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
		colors[ImGuiCol_Tab] = ImVec4(0.13f, 0.13f, 0.13f, 1.00f);
		colors[ImGuiCol_TabHovered] = ImVec4(0.92f, 0.49f, 0.05f, 1.00f);
		colors[ImGuiCol_TabActive] = ImVec4(0.94f, 0.49f, 0.05f, 1.00f);
		colors[ImGuiCol_TabUnfocused] = ImVec4(0.10f, 0.10f, 0.10f, 1.00f);
		colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
		colors[ImGuiCol_DockingPreview] = ImVec4(0.26f, 0.59f, 0.98f, 0.70f);
		colors[ImGuiCol_DockingEmptyBg] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
		colors[ImGuiCol_PlotLines] = ImVec4(0.40f, 0.39f, 0.38f, 0.63f);
		colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.25f, 1.00f, 0.00f, 1.00f);
		colors[ImGuiCol_PlotHistogram] = ImVec4(0.40f, 0.39f, 0.38f, 0.63f);
		colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.25f, 1.00f, 0.00f, 1.00f);
		colors[ImGuiCol_TableHeaderBg] = ImVec4(0.19f, 0.19f, 0.20f, 1.00f);
		colors[ImGuiCol_TableBorderStrong] = ImVec4(0.31f, 0.31f, 0.35f, 1.00f);
		colors[ImGuiCol_TableBorderLight] = ImVec4(0.23f, 0.23f, 0.25f, 1.00f);
		colors[ImGuiCol_TableRowBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
		colors[ImGuiCol_TableRowBgAlt] = ImVec4(1.00f, 1.00f, 1.00f, 0.06f);
		colors[ImGuiCol_TextSelectedBg] = ImVec4(0.25f, 1.00f, 0.00f, 0.43f);
		colors[ImGuiCol_DragDropTarget] = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
		colors[ImGuiCol_NavHighlight] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
		colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
		colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
		colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);
	}

	void ui::injection()
	{
		element::set_position(-80, -50);
		element::texture("##Logo", (PVOID)get_renderer()->logoImage, ImVec2(550, 370));
		element::set_position(40, 320);
		element::combo("Version", get_injection()->m_game_versions[get_injection()->m_type].data(), [] {
			for (size_t i{}; i != get_injection()->m_game_versions.size(); ++i) {
				element::selectable(get_injection()->m_game_versions[i].data(), get_injection()->m_type == i, [i] {
					get_injection()->m_type = i;
					});
			}
			}, true);
		element::set_position(40, 260);
		element::toggle("Inject On Startup", "Active or Desactive Auto Injection", &get_injection()->m_inject_on_launch);
		element::set_position(40, 290);
		element::int_slider("Delay", get_injection()->m_injection_delay, 0, 1000);
		if (get_injection()->is_opened())
		{
			element::set_position(20, 350);
			element::button("Inject", 350, 40, [=] {
				get_injection()->inject("C:\\Virtual X Prep.dll");
				});

			element::set_position(20, 400);
			element::button("Close", 0, 0, [=] {
				get_injection()->close();
				}, true);
			element::button("Restart", 0, 0, [=] {
				get_injection()->restart();
				}, true);
		}
		else
		{
			element::set_position(20, 390);
			if (ImGui::Button("Launch", ImVec2(350, 40)))
			{
				get_injection()->launch_process();
				this->injection_tab = false;
				this->injection_loading_tab = true;
			}
		}
	}

	void ui::injection_loading()
	{
		if (get_injection()->is_opened())
		{
			get_injection()->inject("C:\\Virtual X Prep.dll");
			this->injection_tab = true;
			this->injection_loading_tab = false;

		}
		else
		{
			element::set_position(180, 180);
			ImSpinner::SpinnerRingWatermarks("##spinner", 20, 6, IM_COL32(107, 5, 255, 255));
			element::set_position(190, 250);
			ImSpinner::SpinnerTextFading("##spinner2", "Loading...", 6, 15);
		}
	}

	void ui::auth()
	{
		element::set_position(-80, -50);
		element::texture("##Logo", (PVOID)get_renderer()->logoImage, ImVec2(550, 370));
		element::set_position(20, 250);
		char m_key[52] = "";
		//ImGui::InputTextMultiline("##key", get_api()->m_key, sizeof(get_api()->m_key), ImVec2(350, 40));
		ImGui::InputTextMultiline("##key", m_key, sizeof(m_key), ImVec2(350, 40));
		element::set_position(20, 350);
		element::button("Login", 350, 40, [&] {
			this->injection_tab = true;
			this->auth_tab = false;
			});

		element::button("Logout", 200, 40, [&] {
			//get_api()->deconnexion_api();
			});
	}

	void ui::render()
	{
		element::set_next_window_size(390, 450);
		element::window(NAME, g_running, [] {
			if (get_interface()->auth_tab)
				get_interface()->auth();
			if (get_interface()->injection_tab)
				get_interface()->injection();
			if (get_interface()->injection_loading_tab)
				get_interface()->injection_loading();

			}, ImGuiWindowFlags_NoTitleBar);
	}

	ui* get_interface() { return &g_ui; }
}
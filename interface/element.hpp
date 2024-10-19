#pragma once
#include "../common.hpp"

namespace VX {
	class element
	{
	public:
		static void set_position(float x, float y)
		{
			return ImGui::SetCursorPos(ImVec2(x, y));
		}
		static void window(std::string Title, bool& Value, std::function<void()> Cb = {}, ImGuiWindowFlags Flags = NULL) {
			if (ImGui::Begin(Title.c_str(), &Value, Flags)) {
				if (Cb) {
					Cb();
				}
				ImGui::End();
			}
		}
		static void text(std::string text)
		{
			ImGui::Text(text.c_str());
		}
		static void button(std::string Label, float size_x = 0, float size_y = 0, std::function<void()> Cb = {}, bool ContinueLine = false) {
			if (ImGui::Button(Label.c_str(), ImVec2(size_x, size_y))) {
				if (Cb) {
					Cb();
				}
			}
			if (ContinueLine)
				same_line();
		}
		static void toggle(const char* str_id, const char* tooltip, bool* v)
		{
			ImVec2 p = ImGui::GetCursorScreenPos();
			ImDrawList* draw_list = ImGui::GetWindowDrawList();
			ImGuiContext& g = *GImGui;
			const ImGuiStyle& style = g.Style;

			float height = ImGui::GetFrameHeight() / 1.5;
			float width = height * 2.00f;
			float radius = height * 0.50f;
			float circle = ImGui::GetFrameHeight() / 1.5 * 0.80f;

			if (ImGui::InvisibleButton(str_id, ImVec2(width, height)))
				*v = !*v;
			ImU32 col_bg;
			if (ImGui::IsItemHovered())
			{
				ImGui::SetTooltip(tooltip);
				col_bg = *v ? IM_COL32(107 + 20, 5, 255, 255) : IM_COL32(218 - 20, 218 - 20, 218 - 20, 255);
			}
			else
				col_bg = *v ? IM_COL32(107, 5, 255, 255) : IM_COL32(218, 218, 218, 255);
			const ImRect frame_bb(p, p + ImVec2(width, height));

			ImGui::RenderFrame(frame_bb.Min, frame_bb.Max, col_bg, true, height * 0.5f);
			draw_list->AddRectFilled(p, ImVec2(p.x + width, p.y + height), col_bg, height * 0.5f);
			draw_list->AddCircleFilled(ImVec2(*v ? (p.x + width - radius) : (p.x + radius), p.y + radius), circle - 1.5f, IM_COL32(255, 255, 255, 255));
			ImVec2 label_pos = ImVec2(frame_bb.Max.x + style.ItemInnerSpacing.x + 3, frame_bb.Min.y + style.FramePadding.y - 4);
			ImGui::RenderText(label_pos, str_id);
		}
		static void same_line() {
			ImGui::SameLine();
		}
		static void set_item_focus() {
			ImGui::SetItemDefaultFocus();
		}
		static void set_next_window_size(float x, float y)
		{
			ImGui::SetNextWindowSize(ImVec2(x, y));
		}
		static void combo(std::string Label, std::string Preview, std::function<void()> Cb, bool ContinueLine = false) {
			if (ImGui::BeginCombo(Label.c_str(), Preview.c_str())) {
				Cb();
				ImGui::EndCombo();
			}
			if (ContinueLine)
				same_line();
		}
		static void selectable(std::string Label, bool Selected, std::function<void()> Cb, bool ContinueLine = false) {
			if (ImGui::Selectable(Label.c_str(), Selected)) {
				if (Cb) {
					Cb();
				}
			}
			if (Selected)
				set_item_focus();
			if (ContinueLine)
				same_line();
		}
		static void int_slider(std::string Label, int& Value, int Min, int Max, std::function<void()> Cb = {}, bool ContinueLine = false) {
			if (ImGui::SliderInt(Label.c_str(), &Value, Min, Max)) {
				if (Cb) {
					Cb();
				}
			}
			if (ContinueLine)
				same_line();
		}
		static void texture(const char* str_id, ImTextureID user_texture_id, const ImVec2& size, const ImVec2& uv0 = ImVec2(0, 0), const ImVec2& uv1 = ImVec2(1, 1), const ImVec4& tint_col = ImVec4(1, 1, 1, 1), const ImVec4& border_col = ImVec4(0, 0, 0, 0))
		{
			ImGuiWindow* window = ImGui::GetCurrentWindow();
			if (window->SkipItems)
				return;

			ImRect bb(window->DC.CursorPos, window->DC.CursorPos + size);
			if (border_col.w > 0.0f)
				bb.Max += ImVec2(2, 2);
			ImGui::ItemSize(bb);
			if (!ImGui::ItemAdd(bb, 0))
				return;

			if (border_col.w > 0.0f)
			{
				window->DrawList->AddRect(bb.Min, bb.Max, ImGui::GetColorU32(border_col), 0.0f);
				window->DrawList->AddImage(user_texture_id, bb.Min + ImVec2(1, 1), bb.Max - ImVec2(1, 1), uv0, uv1, ImGui::GetColorU32(tint_col));
			}
			else
			{
				window->DrawList->AddImage(user_texture_id, bb.Min, bb.Max, uv0, uv1, ImGui::GetColorU32(tint_col));
			}
		}
	private:

	};
}
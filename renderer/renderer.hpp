#pragma once
#include "../common.hpp"
#include "../renderer/shader/shader.hpp"
#include "../interface/Interface.hpp"

namespace VX {
	class renderer {
	public:
		bool initialize();
		void cleanup();
	public:
		bool create_dx(HWND hWnd);
		void cleanup_dx();
		void create_render_targets();
		void cleanup_render_targets();
		static void present();
		void helper_render();
		static LRESULT WINAPI wnd_proc(HWND Hwnd, UINT Msg, WPARAM wParam, LPARAM lParam);
	public:
		ImFont* m_Font{};
	public:
		ImFontConfig m_Config{};
	public:
		ID3D11Device* m_Device;
		ID3D11DeviceContext* m_Context;
		IDXGISwapChain* m_SwapChain;
		ID3D11RenderTargetView* m_TargetView;
		ID3D11BlendState* m_BlendState;
		DXGI_SWAP_CHAIN_DESC m_Desc;
		HWND m_Hwnd;
		WNDCLASSEX m_WndClassEx;
		ImVec2 m_WindowSize{ 200, 200 };
		ID3D11ShaderResourceView* logoImage = nullptr;
	};
	renderer* get_renderer();
}
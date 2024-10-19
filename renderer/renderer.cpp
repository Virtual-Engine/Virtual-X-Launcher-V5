#define _CRT_SECURE_NO_WARNINGS
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../STB/STBImage.hpp"
#include "../STB/STBImageWrite.hpp"
#include "renderer.hpp"
#include "../common.hpp"
#include "shader/logo.hpp"
#include <D3DX11tex.h>
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace VX {
	renderer g_renderer;

	bool renderer::initialize() {
		m_WndClassEx = { sizeof(WNDCLASSEX), CS_CLASSDC, wnd_proc, 0L, 0L, GetModuleHandleA(0), 0, 0, 0, 0, NAME, 0 };
		RegisterClassExA(&m_WndClassEx);
		m_Hwnd = CreateWindowExA(WS_EX_TRANSPARENT | WS_EX_LAYERED, m_WndClassEx.lpszClassName, NAME, WS_DISABLED, 200, 200, 0, 0, NULL, NULL, m_WndClassEx.hInstance, NULL);
		if (!create_dx(m_Hwnd)) {
			cleanup_dx();
			UnregisterClassA(m_WndClassEx.lpszClassName, m_WndClassEx.hInstance);
		}
		SetWindowLongA(m_Hwnd, GWL_EXSTYLE, GetWindowLong(m_Hwnd, GWL_EXSTYLE) | WS_EX_LAYERED);
		SetLayeredWindowAttributes(m_Hwnd, RGB(0, 0, 0), 255, LWA_ALPHA);
		ShowWindow(m_Hwnd, SW_HIDE);
		UpdateWindow(m_Hwnd);
		ImGui::CreateContext();
		ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
		ImGui_ImplWin32_Init(m_Hwnd);
		ImGui_ImplDX11_Init(m_Device, m_Context);
		m_Config.FontDataOwnedByAtlas = false;
		ImFontAtlas* Atlas{ ImGui::GetIO().Fonts };
		D3DX11_IMAGE_LOAD_INFO info;
		ID3DX11ThreadPump* pump{ nullptr };
		D3DX11CreateShaderResourceViewFromMemory(m_Device, logo_data, sizeof(logo_data), &info, pump, &logoImage, 0);
		GetShader()->CreateImFont(Atlas, m_Font, fs::path("C:\\Windows\\Fonts\\").append("Arial.ttf"), 19.f, &m_Config);
		ImGui::GetIO().Fonts->AddFontDefault();
		get_interface()->update();
		LOG(INFO) << "renderer initialized";
		return true;
	}
	void renderer::cleanup() {
		ImGui_ImplDX11_Shutdown();
		ImGui_ImplWin32_Shutdown();
		ImGui::DestroyContext();
		cleanup_dx();
		DestroyWindow(m_Hwnd);
		UnregisterClassA(m_WndClassEx.lpszClassName, m_WndClassEx.hInstance);
	}
	void renderer::present() {
		ImGui_ImplDX11_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();
		get_interface()->render();
		ImGui::EndFrame();
		ImGui::Render();
		get_renderer()->helper_render();
	}
	void renderer::helper_render()
	{
		ImVec4 clear_color = ImVec4(0.1f, 0.1f, 0.1f, 0.f);
		const float clear_color_with_alpha[4] = { clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w };
		m_Context->OMSetRenderTargets(1, &m_TargetView, NULL);
		m_Context->ClearRenderTargetView(m_TargetView, clear_color_with_alpha);
		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
		ImGui::UpdatePlatformWindows();
		ImGui::RenderPlatformWindowsDefault();
		m_SwapChain->Present(1, 0);
	}
	LRESULT WINAPI renderer::wnd_proc(HWND Hwnd, UINT Msg, WPARAM wParam, LPARAM lParam) {
		if (ImGui_ImplWin32_WndProcHandler(Hwnd, Msg, wParam, lParam))
			return true;
		if (Msg == WM_SIZE && get_renderer()->m_Device != NULL && wParam != SIZE_MINIMIZED) {
			get_renderer()->cleanup_render_targets();
			get_renderer()->m_SwapChain->ResizeBuffers(0, UINT(LOWORD(lParam)), UINT(HIWORD(lParam)), DXGI_FORMAT_UNKNOWN, 0);
			get_renderer()->create_render_targets();
			return 0;
		}
		else if (Msg == WM_DESTROY) {
			PostQuitMessage(0);
			return 0;
		}
		return DefWindowProc(Hwnd, Msg, wParam, lParam);
	}
	bool renderer::create_dx(HWND hWnd) {
		//Setup the sc
		DXGI_SWAP_CHAIN_DESC sd;
		ZeroMemory(&sd, sizeof(sd));
		sd.BufferCount = 2;
		sd.BufferDesc.Width = 0;
		sd.BufferDesc.Height = 0;
		sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		sd.BufferDesc.RefreshRate.Numerator = 60;
		sd.BufferDesc.RefreshRate.Denominator = 1;
		sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
		sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		sd.OutputWindow = hWnd;
		sd.SampleDesc.Count = 1;
		sd.SampleDesc.Quality = 0;
		sd.Windowed = TRUE;
		sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

		DXGI_SWAP_CHAIN_DESC Desc = {
			{ 0, 0, { 60, 1 }, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED, DXGI_MODE_SCALING_UNSPECIFIED },
			{ 1, 0 },
			DXGI_USAGE_RENDER_TARGET_OUTPUT,
			2,
			hWnd,
			TRUE,
			DXGI_SWAP_EFFECT_DISCARD,
			DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH
		};
		UINT DeviceCreationFlags{};
		D3D_FEATURE_LEVEL FeatureLevel{};
		const D3D_FEATURE_LEVEL FeatureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0 };
		HRESULT res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, DeviceCreationFlags, FeatureLevelArray, 2, D3D11_SDK_VERSION, &sd, &m_SwapChain, &m_Device, &FeatureLevel, &m_Context);
		if (res == DXGI_ERROR_UNSUPPORTED) // Try high-performance WARP software driver if hardware is not available.
			res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_WARP, nullptr, DeviceCreationFlags, FeatureLevelArray, 2, D3D11_SDK_VERSION, &sd, &m_SwapChain, &m_Device, &FeatureLevel, &m_Context);
		if (res != S_OK)
			return false;
		create_render_targets();
		return true;
	}
	void renderer::cleanup_dx() {
		cleanup_render_targets();
		if (m_SwapChain) {
			m_SwapChain->Release();
			m_SwapChain = nullptr;
		}
		if (m_BlendState) {
			m_BlendState->Release();
			m_BlendState = nullptr;
		}
		if (m_Device) {
			m_Device->Release();
			m_Device = nullptr;
		}
		if (m_Context) {
			m_Context->Release();
			m_Context = nullptr;
		}
	}
	void renderer::create_render_targets() {
		ID3D11Texture2D* BackBuffer;
		m_SwapChain->GetBuffer(0, IID_PPV_ARGS(&BackBuffer));
		m_Device->CreateRenderTargetView(BackBuffer, NULL, &m_TargetView);
		BackBuffer->Release();
	}
	void renderer::cleanup_render_targets() {
		if (m_TargetView) {
			m_TargetView->Release();
			m_TargetView = NULL;
		}
	}
	renderer* get_renderer() { return &g_renderer; }
}
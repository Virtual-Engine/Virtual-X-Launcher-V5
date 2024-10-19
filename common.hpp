#pragma once
#define _CRT_SECURE_NO_WARNINGS
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#define _WIN32_WINNT _WIN32_WINNT_WIN7

#pragma disable(warning : 4996)
#include <winsdkver.h>
#include <sdkddkver.h>
#include <windows.h>
#include <winternl.h>
#include <d3d11.h>
#include <Psapi.h>
#include <tlhelp32.h>
#include <cassert>
#include <cinttypes>
#include <cstdarg>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <algorithm>
#include <atomic>
#include <chrono>
#include <deque>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iomanip>
#include <iostream>
#include <limits>
#include <memory>
#include <mutex>
#include <optional>
#include <sstream>
#include <stack>
#include <string>
#include <thread>
#include <type_traits>
#include <map>
#include <unordered_map>
#include <vector>
#include <array>
#include <wrl/client.h>
#include <random>
#include <shellapi.h>
#include <ShlObj_core.h>

#define IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_NOTIFY

#include "ImGui/imgui.h"
#include "ImGui/imgui_internal.h"
#include "ImGui/imgui_impl_win32.h"
#include "ImGui/imgui_impl_dx11.h"
#include "ImGui/toggle/imgui_toggle.h"
#include "ImGui/toggle/imgui_toggle_presets.h"

#include <dxgi.h>
#include <d3d11.h>

#define NAME "Launcher"
#define VERSION "1.0.0"
#define PROCESS "GTA5.exe"

#include "renderer/Renderer.hpp"
#include "libraries/OwnLogger/OwnLogger.hpp"

#include "libraries/curl/curl.h"
#include "libraries/JSON/json.hpp"

#include "Discord/Backend/Handler.hpp"

#pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")

namespace VX {
	typedef const char* String;

	using namespace std::chrono;
	using namespace std::chrono_literals;

	template <typename type>
	using ComPtr = Microsoft::WRL::ComPtr<type>;
	template <typename t> requires std::is_function_v<t>
	using FnPtr = t(*);
	namespace fs = std::filesystem;
	inline bool g_running = true;

	inline MSG g_Msg{};

	inline std::unique_ptr<OwnLogger> g_logger;


	union VectorCoord {
		int16_t i16;
		uint16_t u16;
		int32_t i32;
		uint32_t u32;
		int32_t i64;
		uint32_t u64;
		float fPoint;
	};
	class Vec2 {
	public:
		VectorCoord x{}, y{};
	};
	class Image {
	public:
		unsigned char* Data;
		Vec2 Size;
	};
	class ImageData {
	public:
		unsigned char* Bytes;
		int Size;
	};
	class ShaderData {
	public:
		ID3D11Resource* Resource{};
		ID3D11ShaderResourceView* ResourceView{};
	};
}
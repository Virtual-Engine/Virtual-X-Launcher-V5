#include "injection.hpp"
#include "../../fiber/queue.hpp"
#include <shellapi.h>

namespace VX {
	injection g_injection;

	void injection::inject(std::string path)
	{
		HANDLE Handle{ get_handle(get_id("grcWindow")) };
		LPVOID Mem{ VirtualAllocEx(Handle, nullptr, MAX_PATH, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE) };
		if (!Mem) {
			return;
		}
		if (!WriteProcessMemory(Handle, Mem, path.c_str(), MAX_PATH, nullptr)) {
			LOG(WARNING) << "Failed to write dll to memory";
			return;
		}
		HANDLE Thread{ CreateRemoteThread(Handle, nullptr, NULL, LPTHREAD_START_ROUTINE(LoadLibraryA), Mem, NULL, nullptr) };
		if (!Thread) {
			LOG(WARNING) << "Failed to create remote thread";
			return;
		}
		CloseHandle(Handle);
		if (!Mem) {
			VirtualFreeEx(Handle, Mem, 0, MEM_RELEASE);
		}
	}

	bool injection::launch_version() {
		switch (m_type) {
		case 0: {
			HKEY Key{ read_key(HK_LOCAL_MACHINE, "SOFTWARE\\WOW6432Node\\Rockstar Games\\Launcher", 0, READ) };
			if (Key) {
				const char* RGSPath{ (char*)read_value(Key, "InstallFolder", REG_SZ) };
				if (RGSPath) {
					LOG(WARNING) << "Launching GTA5.exe...";
				}
				else {
					LOG(WARNING) << "Failed to launch GTA using " << m_game_versions[m_type].data();
					m_type++;
					return false;
				}
			}
			else {
				LOG(WARNING) << "Failed to find " << m_game_versions[m_type].data();
				m_type++;
				return false;
			}
		} break;
		case 1: {
			int Result{ cmd("steam://rungameid/{}", true, "271590") };
			if (!Result) {
				LOG(WARNING) << "Failed to launch GTA5 using " << m_game_versions[m_type].data();
				m_type++;
				return false;
			}
		} break;
		case 2: {
			int Result{ cmd("com.epicgames.launcher://apps/9d2d0eb64d5c44529cece33fe2a46482?{}&{}", true, "action=launch", "silent=true") };
			if (!Result) {
				LOG(WARNING) << "Failed to launch GTA5 using " << m_game_versions[m_type].data();
				m_type = 0;
				return false;
			}
		} break;
		}
		return true;
	}

	bool injection::is_opened() {
		return FindWindowA("grcWindow", nullptr);
	}

	void injection::launch_process()
	{
		launch_version();
		m_no_fail = false;
		g_queue.add([&] {
			bool Running{ true };
			std::chrono::time_point Start{ std::chrono::steady_clock::now() };
			while (!get_injection()->is_opened() && Running) {
				std::chrono::seconds Elapsed{ std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - Start) };
				if (!get_injection()->m_no_fail && Elapsed.count() >= 27) {
					Running = false;
					LOG(WARNING) << "Failed to launch GTA5 using " << get_injection()->m_game_versions[get_injection()->m_type].data();
				}
				fiber::current()->sleep();
			}
			if (get_injection()->m_inject_on_launch)
				get_injection()->inject("C:\\RDR2.dll");
		});
	}

	void injection::restart() {
		close();
		fiber::current()->sleep(200ms);
		launch_process();
	}

	HANDLE injection::get_handle(int Pid) {
		HANDLE Handle{ OpenProcess(PROCESS_ALL_ACCESS, FALSE, Pid) };
		if (!Handle) {
			LOG(WARNING) << "Failed to open handle";
			return nullptr;
		}
		return Handle;
	}

	HMODULE injection::get_module(const char* name) {
		uint32_t Pid{ get_id("grcWindow") };
		HANDLE Handle{ get_handle(Pid) };
		HMODULE Modules[255]{};
		MODULEINFO ModuleInfo{};
		DWORD CallbackNeeded{};
		if (!K32EnumProcessModulesEx(Handle, Modules, sizeof(Modules), &CallbackNeeded, LIST_MODULES_64BIT)) {
			LOG(WARNING) << "Failed to enumerate modules";
			return nullptr;
		}
		for (int32_t i{}; i != (CallbackNeeded / sizeof(HMODULE)); ++i) {
			char ModNameCharArray[MAX_PATH];
			if (K32GetModuleFileNameExA(Handle, Modules[i], ModNameCharArray, MAX_PATH)) {
				std::string String{ ModNameCharArray };
				if (!Modules[i]) {
					LOG(WARNING) << std::format("Module at index {} is null", i).c_str();
					return nullptr;
				}
				if (!K32GetModuleInformation(Handle, Modules[i], &ModuleInfo, sizeof(ModuleInfo))) {
					LOG(WARNING) << "Failed to get module info";
					return nullptr;
				}
				if (String.compare(name)) {
					CloseHandle(Handle);
					return Modules[i];
				}
			}
		}
		if (Handle)
			CloseHandle(Handle);
		return nullptr;
	}

	uint32_t injection::get_id(const char* Class, const char* Name) {
		HWND Hwnd{ FindWindowA(Class, Name) };
		if (!Hwnd) {
			LOG(WARNING) << "Failed to find " << Class;
			return NULL;
		}
		DWORD Pid{};
		GetWindowThreadProcessId(Hwnd, &Pid);
		return Pid;
	}
	injection* get_injection() { return &g_injection; }
}
#pragma once
#include "../../common.hpp"
#include "../../fiber/fiber.hpp"
#include "registry.hpp"

namespace VX {

	class injection
	{

	public:
		void inject(std::string path);

		bool launch_version();
		void launch_process();
		bool is_opened();
		void restart();

		HANDLE get_handle(int Pid);
		HMODULE get_module(const char* name);
		std::uint32_t get_id(const char* Class, const char* Name = NULL);

	public:
		void terminate(int Code, const char* Class, const char* Name = NULL) {
			uint32_t Pid{ get_id(Class, Name) };
			HANDLE Handle{ get_handle(Pid) };
			TerminateProcess(Handle, Code);
		}

		void close() {
			terminate(0, "grcWindow");
		}

	public:
		bool m_inject_on_launch{};
		int m_injection_delay{};
		bool m_no_fail{};
		int m_type{};

		std::array<std::string_view, 3> m_game_versions{
			"Rockstar Games Launcher",
			"Steam",
			"Epic Games"
		};

	public:
		template <typename ...A>
		int cmd(std::string Command, bool Shell, A&&... Args) {
			if (Shell) {
				return ShellExecuteA(0, "open", std::vformat(Command, std::make_format_args(Args...)).c_str(), NULL, NULL, SW_SHOW) ? 1 : 0;
			}
			else {
				return system(std::vformat(Command, std::make_format_args(Args...)).c_str());
			}
			return NULL;
		}
		bool IsCorrectTargetArchitecture(HANDLE hProc) {
			BOOL bTarget = FALSE;
			if (!IsWow64Process(hProc, &bTarget)) {
				printf("Can't confirm target process architecture: 0x%X\n", GetLastError());
				return false;
			}

			BOOL bHost = FALSE;
			IsWow64Process(GetCurrentProcess(), &bHost);

			return (bTarget == bHost);
		}

		const char* ConvertWCharToConstChar(const WCHAR* wstr) {
			// Trouver la taille nécessaire pour la chaîne char
			int size_needed = WideCharToMultiByte(CP_UTF8, 0, wstr, -1, nullptr, 0, nullptr, nullptr);

			// Allouer de la mémoire pour la chaîne char
			char* strTo = new char[size_needed];

			// Effectuer la conversion de WCHAR à char
			WideCharToMultiByte(CP_UTF8, 0, wstr, -1, strTo, size_needed, nullptr, nullptr);

			return strTo;
		}

		DWORD getProcessId(const char* processName) {
			DWORD processID = 0;
			HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
			if (hSnapshot != INVALID_HANDLE_VALUE) {
				PROCESSENTRY32 pe;
				pe.dwSize = sizeof(PROCESSENTRY32);

				if (Process32First(hSnapshot, &pe)) {
					do {
						if (strcmp(pe.szExeFile, processName) == 0) {
							processID = pe.th32ProcessID;
							break;
						}
					} while (Process32Next(hSnapshot, &pe));
				}
				CloseHandle(hSnapshot);
			}
			return processID;
		}

		void enableDebugPrivilege() {
			HANDLE hToken;
			LUID luid;
			TOKEN_PRIVILEGES tkp;

			OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken);

			LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &luid);

			tkp.PrivilegeCount = 1;
			tkp.Privileges[0].Luid = luid;
			tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

			AdjustTokenPrivileges(hToken, FALSE, &tkp, sizeof(tkp), NULL, NULL);

			CloseHandle(hToken);
		}
	};

	injection* get_injection();
}
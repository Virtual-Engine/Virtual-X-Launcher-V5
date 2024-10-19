#pragma once
#include "../../common.hpp"
#define HK_CLASSES_ROOT 0
#define HK_CURRENT_USER 1
#define HK_LOCAL_MACHINE 2
#define HK_USERS 3
#define HK_PERFORMANCE_DATA 4
#define HK_PERFORMANCE_TEXT 0x50
#define HK_PERFORMANCE_NLSTEXT 0x60
#define READ (STANDARD_RIGHTS_READ | KEY_QUERY_VALUE | KEY_ENUMERATE_SUB_KEYS | KEY_NOTIFY) & (~SYNCHRONIZE)
#define WRITE (STANDARD_RIGHTS_WRITE | KEY_SET_VALUE | KEY_CREATE_SUB_KEY) & (~SYNCHRONIZE)
#define EXECUTE READ & (~SYNCHRONIZE)
#define ALL_ACCESS (STANDARD_RIGHTS_ALL | KEY_QUERY_VALUE | KEY_SET_VALUE | KEY_CREATE_SUB_KEY | KEY_ENUMERATE_SUB_KEYS | KEY_NOTIFY | KEY_CREATE_LINK) & (~SYNCHRONIZE)

namespace VX {
	inline HKEY read_key(uint32_t Type, std::string Path, int Options, int AccessFlag) {
		HKEY RegistryKey{};
		if (RegOpenKeyExA(HKEY(0x80000000ui64 + Type), Path.c_str(), Options, AccessFlag, &RegistryKey) == ERROR_SUCCESS && (&RegistryKey) != nullptr) {
			return RegistryKey;
		}
		throw std::exception("Failed to read registry key");
		return nullptr;
	}
	inline uint8_t* read_value(HKEY Key, const char* Name, uint32_t Type) {
		DWORD NestedType{ Type };
		uint8_t Buffer[MAX_PATH]{};
		DWORD BufferSize{ sizeof(Buffer) };
		RegQueryValueExA(Key, Name, 0, &NestedType, Buffer, &BufferSize);
		if (Buffer) {
			return Buffer;
		}
		throw std::exception("Failed to read registry value");
		return nullptr;
	}
}
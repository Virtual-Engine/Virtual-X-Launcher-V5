#include "../../common.hpp"


#ifdef _WIN64
#define CURRENT_ARCH IMAGE_FILE_MACHINE_AMD64
#else
#define CURRENT_ARCH IMAGE_FILE_MACHINE_I386
#endif

using f_LoadLibraryA = HINSTANCE(WINAPI*)(const char* lpLibFilename);
using f_GetProcAddress = FARPROC(WINAPI*)(HMODULE hModule, LPCSTR lpProcName);
using f_DLL_ENTRY_POINT = BOOL(WINAPI*)(void* hDll, DWORD dwReason, void* pReserved);

struct MANUAL_MAPPING_DATA
{
	f_LoadLibraryA pLoadLibraryA;
	f_GetProcAddress pGetProcAddress;
	BYTE* baseAddress;
	HINSTANCE hMod;
	FARPROC fOffset;
};

#define RELOC_FLAG32(RelInfo) ((RelInfo >> 0x0C) == IMAGE_REL_BASED_HIGHLOW)
#define RELOC_FLAG64(RelInfo) ((RelInfo >> 0x0C) == IMAGE_REL_BASED_DIR64)

#ifdef _WIN64
#define RELOC_FLAG RELOC_FLAG64
#else
#define RELOC_FLAG RELOC_FLAG32
#endif

#pragma runtime_checks( "", off )
#pragma optimize( "", off )
void __stdcall Shellcode(MANUAL_MAPPING_DATA* pData) {
	if (!pData) {
		pData->hMod = (HINSTANCE)0x404040;
		return;
	}

	BYTE* pBase = pData->baseAddress;
	auto* pOpt = &reinterpret_cast<IMAGE_NT_HEADERS*>(pBase +
		reinterpret_cast<IMAGE_DOS_HEADER*>((uintptr_t)pBase)->e_lfanew)->OptionalHeader;

	auto _LoadLibraryA = pData->pLoadLibraryA;
	auto _GetProcAddress = pData->pGetProcAddress;

	BYTE* LocationDelta = pBase - pOpt->ImageBase;
	if (LocationDelta) {
		if (pOpt->DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].Size) {
			auto* pRelocData = reinterpret_cast<IMAGE_BASE_RELOCATION*>(pBase +
				pOpt->DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress);
			const auto* pRelocEnd = reinterpret_cast<IMAGE_BASE_RELOCATION*>(reinterpret_cast<uintptr_t>(pRelocData)
				+ pOpt->DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].Size);
			while (pRelocData < pRelocEnd && pRelocData->SizeOfBlock) {
				UINT AmountOfEntries = (pRelocData->SizeOfBlock - sizeof(IMAGE_BASE_RELOCATION)) / sizeof(WORD);
				WORD* pRelativeInfo = reinterpret_cast<WORD*>(pRelocData + 1);

				for (UINT i = 0; i != AmountOfEntries; ++i, ++pRelativeInfo) {
					if (RELOC_FLAG(*pRelativeInfo)) {
						UINT_PTR* pPatch = reinterpret_cast<UINT_PTR*>(pBase + pRelocData->VirtualAddress +
							((*pRelativeInfo) & 0xFFF));
						*pPatch += reinterpret_cast<UINT_PTR>(LocationDelta);
					}
				}
				pRelocData = reinterpret_cast<IMAGE_BASE_RELOCATION*>(reinterpret_cast<BYTE*>(pRelocData) +
					pRelocData->SizeOfBlock);
			}
		}
	}

	if (pOpt->DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].Size) {
		auto* pImportDescr = reinterpret_cast<IMAGE_IMPORT_DESCRIPTOR*>(pBase + pOpt->DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);
		while (pImportDescr->Name) {
			char* szMod = reinterpret_cast<char*>(pBase + pImportDescr->Name);
			HINSTANCE hDll = _LoadLibraryA(szMod);

			ULONG_PTR* pThunkRef = reinterpret_cast<ULONG_PTR*>(pBase + pImportDescr->OriginalFirstThunk);
			ULONG_PTR* pFuncRef = reinterpret_cast<ULONG_PTR*>(pBase + pImportDescr->FirstThunk);

			if (!pThunkRef)
				pThunkRef = pFuncRef;

			for (; *pThunkRef; ++pThunkRef, ++pFuncRef) {
				if (IMAGE_SNAP_BY_ORDINAL(*pThunkRef)) {
					*pFuncRef = (ULONG_PTR)_GetProcAddress(hDll, reinterpret_cast<char*>(*pThunkRef & 0xFFFF));
				}
				else {
					auto* pImport = reinterpret_cast<IMAGE_IMPORT_BY_NAME*>(pBase + (*pThunkRef));
					*pFuncRef = (ULONG_PTR)_GetProcAddress(hDll, (char*)pImport->Name);
				}
			}
			++pImportDescr;
		}
	}

	if (pOpt->DataDirectory[IMAGE_DIRECTORY_ENTRY_TLS].Size) {
		auto* pTLS = reinterpret_cast<IMAGE_TLS_DIRECTORY*>(pBase + pOpt->DataDirectory[IMAGE_DIRECTORY_ENTRY_TLS].VirtualAddress);
		auto* pCallback = reinterpret_cast<PIMAGE_TLS_CALLBACK*>(pTLS->AddressOfCallBacks);
		for (; pCallback && *pCallback; ++pCallback)
			(*pCallback)(pBase, DLL_PROCESS_ATTACH, nullptr);
	}

	if (pData->fOffset) {
#ifdef _WIN64
		((FARPROC)((long long)pData->fOffset + (long long)pBase))();
#else 
		((FARPROC)((long)pData->fOffset + (long)pBase))();
#endif
	}
	else {
		auto _DllMain = reinterpret_cast<f_DLL_ENTRY_POINT>(pBase + pOpt->AddressOfEntryPoint);
		_DllMain(pBase, DLL_PROCESS_ATTACH, 0);
	}

	pData->hMod = (HINSTANCE)0x13371337;
}

bool ManualMapDll(HANDLE hProc, BYTE* pSrcData, SIZE_T FileSize, FARPROC fOffset,
	bool ClearHeader = true, bool ClearNonNeededSections = true,
	bool AdjustProtections = true) {
	IMAGE_NT_HEADERS* peHeader = nullptr;
	IMAGE_FILE_HEADER* coffFileHeader = nullptr;
	IMAGE_OPTIONAL_HEADER* winSpecOptHeader = nullptr;
	BYTE* targetBase = nullptr;

	if (reinterpret_cast<IMAGE_DOS_HEADER*>(pSrcData)->e_magic != 0x5A4D) { //"MZ"
		LOG(WARNING) << "Invalid file";
		return false;
	}

	peHeader = reinterpret_cast<IMAGE_NT_HEADERS*>(pSrcData + reinterpret_cast<IMAGE_DOS_HEADER*>(pSrcData)->e_lfanew);
	coffFileHeader = &peHeader->FileHeader;
	winSpecOptHeader = &peHeader->OptionalHeader;

	if (coffFileHeader->Machine != CURRENT_ARCH) {
		LOG(WARNING) << "Invalid platform";
		return false;
	}

	printf("File ok\n");

	targetBase = reinterpret_cast<BYTE*>(VirtualAllocEx(hProc, nullptr, winSpecOptHeader->SizeOfImage, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE));
	if (!targetBase) {
		LOG(WARNING) << "Target process memory allocation failed (ex) 0x" << GetLastError();
		return false;
	}

	DWORD oldp = 0;
	VirtualProtectEx(hProc, targetBase, winSpecOptHeader->SizeOfImage, PAGE_EXECUTE_READWRITE, &oldp);

	MANUAL_MAPPING_DATA data{ 0 };
	data.pLoadLibraryA = LoadLibraryA;
	data.pGetProcAddress = GetProcAddress;
	data.baseAddress = targetBase;
	data.fOffset = fOffset;

	//File header
	if (!WriteProcessMemory(hProc, targetBase, pSrcData, 0x1000, nullptr)) {
		LOG(WARNING) << "Can't write file header 0x" << GetLastError();
		VirtualFreeEx(hProc, targetBase, 0, MEM_RELEASE);
		return false;
	}

	IMAGE_SECTION_HEADER* pSectionHeader = IMAGE_FIRST_SECTION(peHeader);

	for (UINT i = 0; i != coffFileHeader->NumberOfSections; ++i, ++pSectionHeader) {
		if (pSectionHeader->SizeOfRawData) {
			if (!WriteProcessMemory(hProc, targetBase + pSectionHeader->VirtualAddress, pSrcData + pSectionHeader->PointerToRawData, pSectionHeader->SizeOfRawData, nullptr)) {
				LOG(WARNING) << "Can't map sections: 0x" << GetLastError();
				VirtualFreeEx(hProc, targetBase, 0, MEM_RELEASE);
				return false;
			}
		}
	}

	//Mapping params
	BYTE* MappingDataAlloc = reinterpret_cast<BYTE*>(VirtualAllocEx(hProc, nullptr, sizeof(MANUAL_MAPPING_DATA), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE));
	if (!MappingDataAlloc) {
		LOG(WARNING) << "Target process mapping allocation failed (ex) 0x" << GetLastError();
		VirtualFreeEx(hProc, targetBase, 0, MEM_RELEASE);
		return false;
	}

	if (!WriteProcessMemory(hProc, MappingDataAlloc, &data, sizeof(MANUAL_MAPPING_DATA), nullptr)) {
		LOG(WARNING) << "Can't write mapping 0x" << GetLastError();
		VirtualFreeEx(hProc, targetBase, 0, MEM_RELEASE);
		VirtualFreeEx(hProc, MappingDataAlloc, 0, MEM_RELEASE);
		return false;
	}

	//Shell code
	void* pShellcode = VirtualAllocEx(hProc, nullptr, 0x1000, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
	if (!pShellcode) {
		LOG(WARNING) << "Memory shellcode allocation failed (ex) 0x" << GetLastError();
		VirtualFreeEx(hProc, targetBase, 0, MEM_RELEASE);
		VirtualFreeEx(hProc, MappingDataAlloc, 0, MEM_RELEASE);
		return false;
	}

	if (!WriteProcessMemory(hProc, pShellcode, (LPCVOID)Shellcode, 0x1000, nullptr)) {
		LOG(WARNING) << "Can't write shellcode 0x" << GetLastError();
		VirtualFreeEx(hProc, targetBase, 0, MEM_RELEASE);
		VirtualFreeEx(hProc, MappingDataAlloc, 0, MEM_RELEASE);
		VirtualFreeEx(hProc, pShellcode, 0, MEM_RELEASE);
		return false;
	}

#ifdef _DEBUG
	LOG(WARNING) << "My shellcode pointer " << Shellcode;
	LOG(WARNING) << "Target point " << pShellcode;
	// system("pause");
#endif

	HANDLE hThread = CreateRemoteThread(hProc, nullptr, 0, reinterpret_cast<LPTHREAD_START_ROUTINE>(pShellcode), MappingDataAlloc, 0, nullptr);
	if (!hThread) {
		LOG(WARNING) << "Thread creation failed 0x" << GetLastError();
		VirtualFreeEx(hProc, targetBase, 0, MEM_RELEASE);
		VirtualFreeEx(hProc, MappingDataAlloc, 0, MEM_RELEASE);
		VirtualFreeEx(hProc, pShellcode, 0, MEM_RELEASE);
		return false;
	}
	CloseHandle(hThread);

	LOG(WARNING) << "Thread created at: " << pShellcode << ", waiting for return...";

	HINSTANCE hCheck = NULL;
	while (!hCheck) {
		DWORD exitcode = 0;
		GetExitCodeProcess(hProc, &exitcode);
		if (exitcode != STILL_ACTIVE) {
			LOG(WARNING) << "Process crashed, exit code: " << exitcode;
			return false;
		}

		MANUAL_MAPPING_DATA data_checked{ 0 };
		ReadProcessMemory(hProc, MappingDataAlloc, &data_checked, sizeof(data_checked), nullptr);
		hCheck = data_checked.hMod;

		if (hCheck == (HINSTANCE)0x404040) {
			LOG(WARNING) << "Wrong mapping ptr";
			VirtualFreeEx(hProc, targetBase, 0, MEM_RELEASE);
			VirtualFreeEx(hProc, MappingDataAlloc, 0, MEM_RELEASE);
			VirtualFreeEx(hProc, pShellcode, 0, MEM_RELEASE);
			return false;
		}
		else if (hCheck == (HINSTANCE)0x505050) {
			LOG(WARNING) << "Exception support failed!";
		}

		Sleep(10);
	}

	BYTE* emptyBuffer = (BYTE*)malloc(1024 * 1024 * 20);
	if (emptyBuffer == nullptr) {
		LOG(WARNING) << "Unable to allocate memory";
		return false;
	}
	memset(emptyBuffer, 0, 1024 * 1024 * 20);

	//CLEAR PE HEAD
	if (ClearHeader) {
		if (!WriteProcessMemory(hProc, targetBase, emptyBuffer, 0x1000, nullptr)) {
			LOG(WARNING) << "Can't clear HEADER";
		}
	}
	//END CLEAR PE HEAD


	if (ClearNonNeededSections) {
		pSectionHeader = IMAGE_FIRST_SECTION(peHeader);
		for (UINT i = 0; i != coffFileHeader->NumberOfSections; ++i, ++pSectionHeader) {
			if (pSectionHeader->Misc.VirtualSize) {
				if (strcmp((char*)pSectionHeader->Name, ".pdata") == 0 ||
					strcmp((char*)pSectionHeader->Name, ".rsrc") == 0 ||
					strcmp((char*)pSectionHeader->Name, ".reloc") == 0) {

					LOG(WARNING) << "Processing " << pSectionHeader->Name << "removal";

					if (!WriteProcessMemory(hProc, targetBase + pSectionHeader->VirtualAddress,
						emptyBuffer, pSectionHeader->Misc.VirtualSize, nullptr)) {
						LOG(WARNING) << "Can't clear section " << pSectionHeader->Name << GetLastError();
					}
				}
			}
		}
	}

	if (AdjustProtections) {
		pSectionHeader = IMAGE_FIRST_SECTION(peHeader);
		for (UINT i = 0; i != coffFileHeader->NumberOfSections; ++i, ++pSectionHeader) {
			if (pSectionHeader->Misc.VirtualSize) {
				DWORD old = 0;
				DWORD newP = PAGE_READONLY;

				if ((pSectionHeader->Characteristics & IMAGE_SCN_MEM_WRITE) > 0) {
					newP = PAGE_READWRITE;
				}
				else if ((pSectionHeader->Characteristics & IMAGE_SCN_MEM_EXECUTE) > 0) {
					newP = PAGE_EXECUTE_READ;
				}
				if (VirtualProtectEx(hProc, targetBase + pSectionHeader->VirtualAddress, pSectionHeader->Misc.VirtualSize, newP, &old)) {
					LOG(WARNING) << "section " << (char*)pSectionHeader->Name << " set as " << newP;
				}
				else {
					printf("FAIL: section %s not set as %lX\n", (char*)pSectionHeader->Name, newP);
				}
			}
		}
		DWORD old = 0;
		VirtualProtectEx(hProc, targetBase, IMAGE_FIRST_SECTION(peHeader)->VirtualAddress, PAGE_READONLY, &old);
	}

	if (!WriteProcessMemory(hProc, pShellcode, emptyBuffer, 0x1000, nullptr)) {
		LOG(WARNING) << "Can't clear shellcode";
	}
	if (!VirtualFreeEx(hProc, pShellcode, 0, MEM_RELEASE)) {
		LOG(WARNING) << "can't release shell code memory";
	}
	if (!VirtualFreeEx(hProc, MappingDataAlloc, 0, MEM_RELEASE)) {
		LOG(WARNING) << "can't release mapping data memory";
	}

	return true;
}
// made by ducks 
constexpr uintptr_t Bitmap = 0x283C90;

DWORD GetTargetPID(const char* processName) {
	DWORD pid = 0;
	HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

	if (snapshot != INVALID_HANDLE_VALUE) {
		PROCESSENTRY32 entry = { sizeof(entry) };

		if (Process32First(snapshot, &entry)) {
			do {
				if (_stricmp(entry.szExeFile, processName) == 0) {
					pid = entry.th32ProcessID;
					break;
				}
			} while (Process32Next(snapshot, &entry));
		}

		CloseHandle(snapshot);
	}

	return pid;
}

uintptr_t GetModuleBaseAddress(DWORD pid, const char* moduleName) {
	MODULEENTRY32 mod = { sizeof(mod) };
	HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, pid);

	if (snapshot != INVALID_HANDLE_VALUE) {
		if (Module32First(snapshot, &mod)) {
			do {
				if (_stricmp(mod.szModule, moduleName) == 0) {
					CloseHandle(snapshot);
					return reinterpret_cast<uintptr_t>(mod.modBaseAddr);
				}
			} while (Module32Next(snapshot, &mod));
		}
		CloseHandle(snapshot);
	}

	return 0;
}
void PatchCFGExternal(HANDLE hProcess) {
	DWORD pid = GetProcessId(hProcess);
	uintptr_t base = GetModuleBaseAddress(pid, "RobloxPlayerBeta.dll");

	if (!base) {
	//	MessageBoxA(NULL, "Failed to get base address.", "Injector", MB_ICONERROR);
		return;
	}

	uintptr_t cfg_ptr_addr = base + Bitmap;

	uintptr_t cfg_bitmap_addr = 0;
	if (!ReadProcessMemory(hProcess, (LPCVOID)cfg_ptr_addr, &cfg_bitmap_addr, sizeof(cfg_bitmap_addr), nullptr) || !cfg_bitmap_addr) {
		//MessageBoxA(NULL, "Failed to read CFG pointer.", "Injector", MB_ICONERROR);
		return;
	}

	SIZE_T bitmap_size = ((0x7FFFFFFFFFFF >> 0x13) + 1);
	uint8_t* patch = new uint8_t[bitmap_size];
	memset(patch, 0xFF, bitmap_size);

	if (!WriteProcessMemory(hProcess, (LPVOID)cfg_bitmap_addr, patch, bitmap_size, nullptr)) {
		//MessageBoxA(NULL, "Failed to write CFG memory.", "Injector", MB_ICONERROR);
	}
	else {
		std::cout << std::hex << "Patching Control Flow Guard at 0x" << cfg_bitmap_addr << std::dec << '\n';
	}

	delete[] patch;
}

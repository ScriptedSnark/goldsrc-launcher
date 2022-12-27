#include "MinHook.h"
#include "Utils.hpp"

void HookEngine()
{
	void* handle;
	void* base;
	size_t size;

	if (!MemUtils::GetModuleInfo(L"hw.dll", &handle, &base, &size))
	{
		printf("[hl.exe] Can't get module info about hw.dll! Stopping hooking...\n");
		return;
	}

	Utils utils = Utils::Utils(handle, base, size);
	printf("[hl.exe] Hooked hw.dll!\n");
}

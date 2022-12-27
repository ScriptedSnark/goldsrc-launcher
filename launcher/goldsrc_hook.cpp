#include "goldsrc_hook.h"

#include "MinHook.h"
#include "Utils.hpp"

_SDL_CreateWindow ORIG_SDL_CreateWindow = NULL;
_SDL_GL_SetAttribute ORIG_SDL_GL_SetAttribute = NULL;

SDL_Window* HOOKED_SDL_CreateWindow(const char* title, int x, int y, int w, int h, Uint32 flags)
{
	ORIG_SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
	return ORIG_SDL_CreateWindow(title, x, y, w, h, flags);
}

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

	// Get SDL functions
	HMODULE hSdl2 = GetModuleHandle("SDL2.dll");
	ORIG_SDL_CreateWindow = (_SDL_CreateWindow)GetProcAddress(hSdl2, "SDL_CreateWindow");
	ORIG_SDL_GL_SetAttribute = (_SDL_GL_SetAttribute)GetProcAddress(hSdl2, "SDL_GL_SetAttribute");

	if (ORIG_SDL_CreateWindow && ORIG_SDL_GL_SetAttribute)
		printf("[hl.exe] Got SDL_CreateWindow & SDL_GL_SetAttribute! Setting up stencil buffer...\n");
	else
	{
		printf("[hl.exe] Can't get SDL_CreateWindow & SDL_GL_SetAttribute! There will be no stencil buffer.\n");
		return;
	}

	void* pSDL_CreateWindow = (void*)ORIG_SDL_CreateWindow;
	MH_CreateHook(pSDL_CreateWindow, (void*)HOOKED_SDL_CreateWindow, (void**)&ORIG_SDL_CreateWindow);

	MH_EnableHook(MH_ALL_HOOKS);
}

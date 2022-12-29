#include "goldsrc_hook.h"

#include "MinHook.h"
#include "Utils.hpp"

#include "ImGUI/imgui.h"
#include "ImGUI/imgui_impl_sdl.h"

#include <gl/GL.h>

_SDL_CreateWindow ORIG_SDL_CreateWindow = NULL;
_SDL_GL_SwapWindow ORIG_SDL_GL_SwapWindow = NULL;

SDL_Window* HOOKED_SDL_CreateWindow(const char* title, int x, int y, int w, int h, Uint32 flags)
{
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
	return ORIG_SDL_CreateWindow(title, x, y, w, h, flags);
}

void HOOKED_SDL_GL_SwapWindow(SDL_Window* window)
{
	ImGui_ImplSdl_NewFrame(window);

	//ImGui::ShowTestWindow();

	glViewport(0, 0, (int)ImGui::GetIO().DisplaySize.x, (int)ImGui::GetIO().DisplaySize.y);
	ImGui::Render();

	ORIG_SDL_GL_SwapWindow(window);
}

int ImGUI_ProcessEvent(void* data, SDL_Event* event)
{
	return ImGui_ImplSdl_ProcessEvent(event);
}

void HookSDL2()
{
	// Get SDL functions
	HMODULE hSdl2 = GetModuleHandle("SDL2.dll");
	ORIG_SDL_CreateWindow = (_SDL_CreateWindow)GetProcAddress(hSdl2, "SDL_CreateWindow");
	ORIG_SDL_GL_SwapWindow = (_SDL_GL_SwapWindow)GetProcAddress(hSdl2, "SDL_GL_SwapWindow");

	if (ORIG_SDL_CreateWindow)
		printf("[SDL2.dll] Got SDL_CreateWindow! Setting up stencil buffer...\n");
	else
		printf("[SDL2.dll] Can't get SDL_CreateWindow! There will be no stencil buffer.\n");

	if (ORIG_SDL_GL_SwapWindow)
		printf("[SDL2.dll] Got SDL_GL_SwapWindow! Now you can use ImGUI...\n");
	else
		printf("[SDL2.dll] Can't get SDL_GL_SwapWindow! There will be no ImGUI.\n");

	if (ORIG_SDL_CreateWindow)
	{
		void* pSDL_CreateWindow = (void*)ORIG_SDL_CreateWindow;
		MH_CreateHook(pSDL_CreateWindow, (void*)HOOKED_SDL_CreateWindow, (void**)&ORIG_SDL_CreateWindow);
	}

	if (ORIG_SDL_GL_SwapWindow)
	{
		void* pSDL_GL_SwapWindow = (void*)ORIG_SDL_GL_SwapWindow;
		MH_CreateHook(pSDL_GL_SwapWindow, (void*)HOOKED_SDL_GL_SwapWindow, (void**)&ORIG_SDL_GL_SwapWindow);

		ImGui_ImplSdl_Init(SDL_GetWindowFromID(1));

		SDL_AddEventWatch(ImGUI_ProcessEvent, NULL);
	}
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
}

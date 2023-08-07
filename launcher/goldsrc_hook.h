#ifdef GOLDSRC_HOOK_H_RECURSE_GUARD
#error Recursive header files inclusion detected in goldsrc_hook.h
#else //GOLDSRC_HOOK_H_RECURSE_GUARD

#define GOLDSRC_HOOK_H_RECURSE_GUARD

#ifndef GOLDSRC_HOOK_H_GUARD
#define GOLDSRC_HOOK_H_GUARD
#pragma once

#define Hook(future_name)                                                                                                                  \
	{                                                                                                                                      \
		auto f##future_name = utils.FindAsync(ORIG_##future_name, patterns::engine::future_name);                                          \
		auto pattern = f##future_name.get();                                                                                               \
		if (ORIG_##future_name)                                                                                                            \
		{                                                                                                                                  \
			printf("[hw dll] Found " #future_name " at %p (using the %s pattern).\n", ORIG_##future_name, pattern->name()); \
			void* p##future_name = (void*)ORIG_##future_name;                                                                              \
			MH_CreateHook(p##future_name, (void*)##future_name, (void**)&ORIG_##future_name);                                              \
		}                                                                                                                                  \
		else                                                                                                                               \
		{                                                                                                                                  \
			printf("[hw dll] Could not find " #future_name ".\n");                                                          \
		}                                                                                                                                  \
	}

#include "SDL2/SDL.h"

typedef SDL_Window* (*_SDL_CreateWindow)(const char* title, int x, int y, int w, int h, Uint32 flags);
typedef void (*_SDL_GL_SwapWindow)(SDL_Window* window);

extern _SDL_CreateWindow ORIG_SDL_CreateWindow;
extern _SDL_GL_SwapWindow ORIG_SDL_GL_SwapWindow;

void HookSDL2();
void HookEngine();

#endif //GOLDSRC_HOOK_H_GUARD

#undef GOLDSRC_HOOK_H_RECURSE_GUARD
#endif //GOLDSRC_HOOK_H_RECURSE_GUARD
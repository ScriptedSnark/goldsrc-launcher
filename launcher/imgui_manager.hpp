#ifdef IMGUI_MANAGER_HPP_RECURSE_GUARD
#error Recursive header files inclusion detected in imgui_manager.hpp
#else //IMGUI_MANAGER_HPP_RECURSE_GUARD

#define IMGUI_MANAGER_HPP_RECURSE_GUARD

#ifndef IMGUI_MANAGER_HPP_GUARD
#define IMGUI_MANAGER_HPP_GUARD
#pragma once

#include "ImGUI/imgui.h"
#include "ImGUI/imgui_impl_sdl.h"
#include "ImGUI/imgui_impl_opengl2.h"

class CImGuiManager
{
public:
	void Init();
	void InitBackends(SDL_Window* window);
	void Draw(SDL_Window* window);
};

int ImGui_ProcessEvent(void* data, SDL_Event* event);

#endif //IMGUI_MANAGER_HPP_GUARD

#undef IMGUI_MANAGER_HPP_RECURSE_GUARD
#endif //IMGUI_MANAGER_HPP_RECURSE_GUARD
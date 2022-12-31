#include "imgui_manager.hpp"

#include <Windows.h>
#include <gl/GL.h>
#include "SDL2/SDL_opengl.h"

//-----------------------------------------------------------------------------
// Initialize ImGui by creating context
//-----------------------------------------------------------------------------
void CImGuiManager::Init()
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	(void)io;

	io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;
}

//-----------------------------------------------------------------------------
// Initialize OpenGL2/SDL2 backends of ImGui
//-----------------------------------------------------------------------------
void CImGuiManager::InitBackends(SDL_Window* window)
{
	ImGui_ImplOpenGL2_Init();
	ImGui_ImplSDL2_InitForOpenGL(window, ImGui::GetCurrentContext());

	// Do other things...
}

//-----------------------------------------------------------------------------
// Draw ImGui elements
//-----------------------------------------------------------------------------
void CImGuiManager::Draw(SDL_Window* window)
{
	ImGui_ImplOpenGL2_NewFrame();
	ImGui_ImplSDL2_NewFrame(window);
	ImGui::NewFrame();

	// Here you draw...
	// -------------------------

	ImGui::ShowDemoWindow();

	// -------------------------

	glViewport(0, 0, (int)ImGui::GetIO().DisplaySize.x, (int)ImGui::GetIO().DisplaySize.y);
	ImGui::Render();
	ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());
}

//-----------------------------------------------------------------------------
// ImGui process event for SDL2
//-----------------------------------------------------------------------------
int ImGui_ProcessEvent(void* data, SDL_Event* event)
{
	return ImGui_ImplSDL2_ProcessEvent(event);
}
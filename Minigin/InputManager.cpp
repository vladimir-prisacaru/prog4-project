#include "InputManager.h"

#include <SDL3/SDL.h>

#include <imgui.h>
#include <backends/imgui_impl_sdl3.h>



bool dae::InputManager::ProcessInput()
{
    SDL_Event e;

    while (SDL_PollEvent(&e))
    {
        ImGui_ImplSDL3_ProcessEvent(&e);

        if (e.type == SDL_EVENT_QUIT)
        {
            return false;
        }
        if (e.type == SDL_EVENT_KEY_DOWN)
        {

        }
        if (e.type == SDL_EVENT_MOUSE_BUTTON_DOWN)
        {

        }
    }

    return true;
}
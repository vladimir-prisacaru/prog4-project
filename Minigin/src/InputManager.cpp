#include "InputManager.h"
#include "InputCommand.h"

#include <SDL3/SDL.h>
#include <backends/imgui_impl_sdl3.h>

#include <cassert>

namespace dae
{
    bool InputManager::ProcessInput()
    {
        int numKeys { };
        const bool* currentKeys = SDL_GetKeyboardState(&numKeys);

        if (static_cast<int>(m_PrevKeyboardState.size()) != numKeys)
            m_PrevKeyboardState.assign(numKeys, false);

        SDL_Event e;
        while (SDL_PollEvent(&e))
        {
            ImGui_ImplSDL3_ProcessEvent(&e);
            if (e.type == SDL_EVENT_QUIT)
                return false;
        }

        ProcessKeyboardCommands();

        for (int i = 0; i < numKeys; ++i)
            m_PrevKeyboardState[i] = currentKeys[i];

        for (unsigned int i = 0; i < k_MaxControllers; ++i)
            if (m_GamePads[i]) m_GamePads[i]->ProcessInput();

        return true;
    }

    void InputManager::AddControllerCommand(unsigned int     controllerIndex,
        ControllerButton button,
        KeyState         state,
        std::unique_ptr<InputCommand> command)
    {
        assert(controllerIndex < k_MaxControllers);
        GetOrCreateGamePad(controllerIndex).AddCommand(button, state, std::move(command));
    }

    void InputManager::RemoveControllerCommand(unsigned int     controllerIndex,
        ControllerButton button,
        KeyState         state)
    {
        assert(controllerIndex < k_MaxControllers);
        if (m_GamePads[controllerIndex])
            m_GamePads[controllerIndex]->RemoveCommand(button, state);
    }

    void InputManager::AddKeyboardCommand(SDL_Scancode key,
        KeyState     state,
        std::unique_ptr<InputCommand> command)
    {
        m_KeyboardBindings[{key, state}] = std::move(command);
    }

    void InputManager::RemoveKeyboardCommand(SDL_Scancode key, KeyState state)
    {
        m_KeyboardBindings.erase({ key, state });
    }

    Gamepad& InputManager::GetOrCreateGamePad(unsigned int index)
    {
        if (!m_GamePads[index])
            m_GamePads[index] = std::make_unique<Gamepad>(index);
        return *m_GamePads[index];
    }

    void InputManager::ProcessKeyboardCommands()
    {
        int numKeys { };
        const bool* currentKeys = SDL_GetKeyboardState(&numKeys);

        for (auto& [bindKey, command] : m_KeyboardBindings)
        {
            const auto [scancode, state] = bindKey;
            const int  idx = static_cast<int>(scancode);

            if (idx >= numKeys) continue;

            const bool currDown = currentKeys[idx];
            const bool prevDown = (idx < static_cast<int>(m_PrevKeyboardState.size()))
                && m_PrevKeyboardState[idx];

            bool shouldExecute = false;
            switch (state)
            {
                case KeyState::Down:    shouldExecute = currDown && !prevDown; break;
                case KeyState::Up:      shouldExecute = !currDown && prevDown; break;
                case KeyState::Pressed: shouldExecute = currDown;              break;
            }

            if (shouldExecute)
                command->Execute();
        }
    }
}
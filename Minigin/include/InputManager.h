#pragma once
#include "Singleton.h"
#include "Gamepad.h"

#include <memory>
#include <map>
#include <vector>
#include <SDL3/SDL.h>

namespace dae
{
    class InputCommand;

    class InputManager final
    {
        public:

        explicit InputManager();

        ~InputManager();
        InputManager(const InputManager& other) = delete;
        InputManager(InputManager&& other) = delete;
        InputManager& operator=(const InputManager& other) = delete;
        InputManager& operator=(InputManager&& other) = delete;

        // Main loop, returns false when the app should quit
        bool ProcessInput();

        void AddControllerCommand(unsigned int controllerIndex, ControllerButton button,
            KeyState state, std::unique_ptr<InputCommand> command);

        void RemoveControllerCommand(unsigned int controllerIndex, ControllerButton button,
            KeyState state);

        void AddKeyboardCommand(SDL_Scancode key, KeyState state,
            std::unique_ptr<InputCommand> command);

        void RemoveKeyboardCommand(SDL_Scancode key, KeyState state);



        private:

        static constexpr unsigned int k_MaxControllers { 4 };
        std::unique_ptr<Gamepad> m_GamePads[k_MaxControllers] { };

        std::vector<bool> m_PrevKeyboardState { };

        using KeyboardBindingKey = std::pair<SDL_Scancode, KeyState>;
        std::map<KeyboardBindingKey, std::unique_ptr<InputCommand>> m_KeyboardBindings { };

        Gamepad& GetOrCreateGamePad(unsigned int index);
        void ProcessKeyboardCommands();
    };
}
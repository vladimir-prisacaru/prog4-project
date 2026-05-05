#pragma once

#include "InputCommand.h"

#include <memory>

namespace dae
{
    enum class KeyState
    {
        Down,
        Up,
        Pressed
    };

    enum class ControllerButton
    {
        DPadUp,
        DPadDown,
        DPadLeft,
        DPadRight,
        Start,
        Back,
        LeftThumb,
        RightThumb,
        LeftShoulder,
        RightShoulder,
        ButtonA,
        ButtonB,
        ButtonX,
        ButtonY
    };

    class Gamepad final
    {
        public:
        explicit Gamepad(unsigned int controllerIndex);
        ~Gamepad();

        Gamepad(const Gamepad&) = delete;
        Gamepad(Gamepad&&) = delete;
        Gamepad& operator=(const Gamepad&) = delete;
        Gamepad& operator=(Gamepad&&) = delete;

        // Call once per frame: reads controller state and executes bound commands
        void ProcessInput();

        void AddCommand(ControllerButton button, KeyState state, std::unique_ptr<InputCommand> command);
        void RemoveCommand(ControllerButton button, KeyState state);

        bool IsConnected() const;

        private:

        class GamepadImpl;
        std::unique_ptr<GamepadImpl> m_pImpl;
    };
}
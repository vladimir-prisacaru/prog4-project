#include "Gamepad.h"

#include <map>
#include <memory>

using namespace dae;



#ifdef __EMSCRIPTEN__



#include <SDL3/SDL.h>

static SDL_GamepadButton ToSDLButton(ControllerButton button)
{
    switch (button)
    {
        case ControllerButton::DPadUp:        return SDL_GAMEPAD_BUTTON_DPAD_UP;
        case ControllerButton::DPadDown:      return SDL_GAMEPAD_BUTTON_DPAD_DOWN;
        case ControllerButton::DPadLeft:      return SDL_GAMEPAD_BUTTON_DPAD_LEFT;
        case ControllerButton::DPadRight:     return SDL_GAMEPAD_BUTTON_DPAD_RIGHT;
        case ControllerButton::Start:         return SDL_GAMEPAD_BUTTON_START;
        case ControllerButton::Back:          return SDL_GAMEPAD_BUTTON_BACK;
        case ControllerButton::LeftThumb:     return SDL_GAMEPAD_BUTTON_LEFT_STICK;
        case ControllerButton::RightThumb:    return SDL_GAMEPAD_BUTTON_RIGHT_STICK;
        case ControllerButton::LeftShoulder:  return SDL_GAMEPAD_BUTTON_LEFT_SHOULDER;
        case ControllerButton::RightShoulder: return SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER;
        case ControllerButton::ButtonA:       return SDL_GAMEPAD_BUTTON_SOUTH;
        case ControllerButton::ButtonB:       return SDL_GAMEPAD_BUTTON_EAST;
        case ControllerButton::ButtonX:       return SDL_GAMEPAD_BUTTON_WEST;
        case ControllerButton::ButtonY:       return SDL_GAMEPAD_BUTTON_NORTH;
        default:                              return SDL_GAMEPAD_BUTTON_INVALID;
    }
}

class Gamepad::GamepadImpl final
{
    public:

    explicit GamepadImpl(unsigned int controllerIndex)
        : m_ControllerIndex { controllerIndex }
    {
        std::fill(std::begin(m_CurrentButtons), std::end(m_CurrentButtons), false);
        std::fill(std::begin(m_PreviousButtons), std::end(m_PreviousButtons), false);

        TryOpen();
    }

    ~GamepadImpl()
    {
        if (m_pGamepad) { SDL_CloseGamepad(m_pGamepad); m_pGamepad = nullptr; }
    }

    void Update()
    {
        std::copy(std::begin(m_CurrentButtons), std::end(m_CurrentButtons),
            std::begin(m_PreviousButtons));

        if (!m_pGamepad) TryOpen();

        if (!m_pGamepad) { m_IsConnected = false; return; }

        m_IsConnected = SDL_GamepadConnected(m_pGamepad);
        if (!m_IsConnected)
        {
            SDL_CloseGamepad(m_pGamepad);
            m_pGamepad = nullptr;
            std::fill(std::begin(m_CurrentButtons), std::end(m_CurrentButtons), false);
            return;
        }

        for (int i = 0; i < SDL_GAMEPAD_BUTTON_COUNT; ++i)
            m_CurrentButtons[i] = SDL_GetGamepadButton(m_pGamepad, static_cast<SDL_GamepadButton>(i));
    }

    bool IsDownThisFrame(SDL_GamepadButton btn) const
    {
        const int i = static_cast<int>(btn);
        return m_CurrentButtons[i] && !m_PreviousButtons[i];
    }

    bool IsUpThisFrame(SDL_GamepadButton btn) const
    {
        const int i = static_cast<int>(btn);
        return !m_CurrentButtons[i] && m_PreviousButtons[i];
    }

    bool IsPressed(SDL_GamepadButton btn) const
    {
        return m_CurrentButtons[static_cast<int>(btn)];
    }

    bool IsConnected() const { return m_IsConnected; }

    using BindingKey = std::pair<ControllerButton, KeyState>;
    std::map<BindingKey, std::unique_ptr<InputCommand>> m_Bindings;

    unsigned int m_ControllerIndex { };

    private:

    void TryOpen()
    {
        int count { };
        SDL_JoystickID* ids = SDL_GetGamepads(&count);
        if (ids && m_ControllerIndex < static_cast<unsigned int>(count))
            m_pGamepad = SDL_OpenGamepad(ids[m_ControllerIndex]);
        SDL_free(ids);
    }

    SDL_Gamepad* m_pGamepad { nullptr };
    bool         m_IsConnected { false };
    bool         m_CurrentButtons[SDL_GAMEPAD_BUTTON_COUNT] { };
    bool         m_PreviousButtons[SDL_GAMEPAD_BUTTON_COUNT] { };
};



Gamepad::GamePad(unsigned int controllerIndex)
    : m_pImpl { std::make_unique<GamepadImpl>(controllerIndex) }
{ }

Gamepad::~GamePad() = default;

void Gamepad::ProcessInput()
{
    m_pImpl->Update();
    if (!m_pImpl->IsConnected()) return;

    for (auto& [key, command] : m_pImpl->m_Bindings)
    {
        const auto& [button, state] = key;
        const SDL_GamepadButton sdlBtn = ToSDLButton(button);
        if (sdlBtn == SDL_GAMEPAD_BUTTON_INVALID) continue;

        bool shouldExecute = false;
        switch (state)
        {
            case KeyState::Down:    shouldExecute = m_pImpl->IsDownThisFrame(sdlBtn); break;
            case KeyState::Up:      shouldExecute = m_pImpl->IsUpThisFrame(sdlBtn);   break;
            case KeyState::Pressed: shouldExecute = m_pImpl->IsPressed(sdlBtn);       break;
        }

        if (shouldExecute)
            command->Execute();
    }
}

void Gamepad::AddCommand(ControllerButton button, KeyState state, std::unique_ptr<InputCommand> command)
{
    m_pImpl->m_Bindings[{button, state}] = std::move(command);
}

void Gamepad::RemoveCommand(ControllerButton button, KeyState state)
{
    m_pImpl->m_Bindings.erase({ button, state });
}

bool Gamepad::IsConnected() const { return m_pImpl->IsConnected(); }



#else // !__EMSCRIPTEN__



#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <xinput.h>

static WORD ToXInputButton(ControllerButton button)
{
    switch (button)
    {
        case ControllerButton::DPadUp:        return XINPUT_GAMEPAD_DPAD_UP;
        case ControllerButton::DPadDown:      return XINPUT_GAMEPAD_DPAD_DOWN;
        case ControllerButton::DPadLeft:      return XINPUT_GAMEPAD_DPAD_LEFT;
        case ControllerButton::DPadRight:     return XINPUT_GAMEPAD_DPAD_RIGHT;
        case ControllerButton::Start:         return XINPUT_GAMEPAD_START;
        case ControllerButton::Back:          return XINPUT_GAMEPAD_BACK;
        case ControllerButton::LeftThumb:     return XINPUT_GAMEPAD_LEFT_THUMB;
        case ControllerButton::RightThumb:    return XINPUT_GAMEPAD_RIGHT_THUMB;
        case ControllerButton::LeftShoulder:  return XINPUT_GAMEPAD_LEFT_SHOULDER;
        case ControllerButton::RightShoulder: return XINPUT_GAMEPAD_RIGHT_SHOULDER;
        case ControllerButton::ButtonA:       return XINPUT_GAMEPAD_A;
        case ControllerButton::ButtonB:       return XINPUT_GAMEPAD_B;
        case ControllerButton::ButtonX:       return XINPUT_GAMEPAD_X;
        case ControllerButton::ButtonY:       return XINPUT_GAMEPAD_Y;
        default:                              return 0;
    }
}

class Gamepad::GamepadImpl final
{
    public:

    explicit GamepadImpl(unsigned int controllerIndex)
        : m_ControllerIndex { controllerIndex }
    {
        ZeroMemory(&m_CurrentState, sizeof(XINPUT_STATE));
        ZeroMemory(&m_PreviousState, sizeof(XINPUT_STATE));
    }

    void Update()
    {
        CopyMemory(&m_PreviousState, &m_CurrentState, sizeof(XINPUT_STATE));
        ZeroMemory(&m_CurrentState, sizeof(XINPUT_STATE));

        m_IsConnected = (XInputGetState(m_ControllerIndex, &m_CurrentState) == ERROR_SUCCESS);
        if (!m_IsConnected) return;

        const WORD curr = m_CurrentState.Gamepad.wButtons;
        const WORD prev = m_PreviousState.Gamepad.wButtons;
        const WORD changes = curr ^ prev;

        m_ButtonsPressedThisFrame = changes & curr;
        m_ButtonsReleasedThisFrame = changes & ~curr;
    }

    bool IsDownThisFrame(WORD mask) const { return (m_ButtonsPressedThisFrame & mask) != 0; }
    bool IsUpThisFrame(WORD mask) const { return (m_ButtonsReleasedThisFrame & mask) != 0; }
    bool IsPressed(WORD mask) const { return (m_CurrentState.Gamepad.wButtons & mask) != 0; }

    bool IsConnected() const { return m_IsConnected; }

    using BindingKey = std::pair<ControllerButton, KeyState>;
    std::map<BindingKey, std::unique_ptr<InputCommand>> m_Bindings;

    unsigned int m_ControllerIndex { };



    private:

    XINPUT_STATE m_CurrentState { };
    XINPUT_STATE m_PreviousState { };
    WORD m_ButtonsPressedThisFrame { };
    WORD m_ButtonsReleasedThisFrame { };
    bool m_IsConnected { false };
};



Gamepad::Gamepad(unsigned int controllerIndex)
    : m_pImpl { std::make_unique<GamepadImpl>(controllerIndex) }
{ }

Gamepad::~Gamepad() = default;

void Gamepad::ProcessInput()
{
    m_pImpl->Update();
    if (!m_pImpl->IsConnected()) return;

    for (auto& [key, command] : m_pImpl->m_Bindings)
    {
        const auto& [button, state] = key;
        const WORD mask = ToXInputButton(button);

        bool shouldExecute = false;
        switch (state)
        {
            case KeyState::Down:    shouldExecute = m_pImpl->IsDownThisFrame(mask); break;
            case KeyState::Up:      shouldExecute = m_pImpl->IsUpThisFrame(mask);   break;
            case KeyState::Pressed: shouldExecute = m_pImpl->IsPressed(mask);       break;
        }

        if (shouldExecute)
            command->Execute();
    }
}

void Gamepad::AddCommand(ControllerButton button, KeyState state, std::unique_ptr<InputCommand> command)
{
    m_pImpl->m_Bindings[{button, state}] = std::move(command);
}

void Gamepad::RemoveCommand(ControllerButton button, KeyState state)
{
    m_pImpl->m_Bindings.erase({ button, state });
}

bool Gamepad::IsConnected() const { return m_pImpl->IsConnected(); }



#endif // __EMSCRIPTEN__
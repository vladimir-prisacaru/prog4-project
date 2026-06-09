#include "Player.h"
#include "InputManager.h"
#include "Scene.h"

namespace dae
{
    void Player::Register()
    {
        RegisterComponent<Player>("player");

        RegisterParameter("player_id", &Player::m_PlayerId);
        RegisterParameter("move_speed", &Player::m_MoveSpeed);
    }

    void Player::OnInit(EngineCtx& ctx)
    {
        InputManager* input { ctx.inputManager };

        // Set up move input commands
        input->AddControllerCommand(m_PlayerId, ControllerButton::DPadDown,
            KeyState::Pressed, std::make_unique<PlayerMoveCommand>(this, glm::vec2 { 0.0f, 1.0f }));

        input->AddControllerCommand(m_PlayerId, ControllerButton::DPadUp,
            KeyState::Pressed, std::make_unique<PlayerMoveCommand>(this, glm::vec2 { 0.0f, -1.0f }));

        input->AddControllerCommand(m_PlayerId, ControllerButton::DPadRight,
            KeyState::Pressed, std::make_unique<PlayerMoveCommand>(this, glm::vec2 { 1.0f, 0.0f }));

        input->AddControllerCommand(m_PlayerId, ControllerButton::DPadLeft,
            KeyState::Pressed, std::make_unique<PlayerMoveCommand>(this, glm::vec2 { -1.0f, 0.0f }));

        // Get the tunnel component
    }

    void Player::Update(EngineCtx& ctx)
    {
        if (glm::length(m_InputMoveDir) < 0.01f)
            return;

        // Consume input direction
        glm::vec2 input = glm::normalize(m_InputMoveDir);
        m_InputMoveDir = glm::vec2(0.0f, 0.0f);

        auto& transform { GetTransform() };
        auto pos { transform.GetLocalPos() };
        transform.SetLocalPos(pos + (input * m_MoveSpeed * ctx.deltaTime));
    }

    void Player::OnDestroy(EngineCtx& ctx)
    {
        InputManager* input { ctx.inputManager };

        // Remove move input commands
        input->RemoveControllerCommand(m_PlayerId, ControllerButton::DPadDown,
            KeyState::Pressed);

        input->RemoveControllerCommand(m_PlayerId, ControllerButton::DPadUp,
            KeyState::Pressed);

        input->RemoveControllerCommand(m_PlayerId, ControllerButton::DPadRight,
            KeyState::Pressed);

        input->RemoveControllerCommand(m_PlayerId, ControllerButton::DPadLeft,
            KeyState::Pressed);
    }

    void Player::OnOverlap(ICollider*)
    {

    }

    void Player::OnOverlapEnd(ICollider*)
    {

    }

    void Player::SetMoveDir(glm::vec2 moveDir)
    {
        // Only set if the input was consumed already
        if (glm::length(m_InputMoveDir) > 0.01f)
            return;

        m_InputMoveDir = moveDir;
    }

    void PlayerMoveCommand::Execute()
    {
        m_Player->SetMoveDir(m_Direction);
    }
}
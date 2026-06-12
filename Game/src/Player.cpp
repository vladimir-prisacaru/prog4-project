#include "Player.h"
#include "InputManager.h"
#include "SceneManager.h"
#include "TunnelComponent.h"
#include "SpriteComponent.h"
#include "AttackComponent.h"
#include "Enemy.h"
#include "EventManager.h"

#include <array>

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
        SetupInput(ctx.inputManager);

        // Get the tunnel component
        m_Tunnel = ctx.sceneManager->GetFirstComponentByType<TunnelComponent>();
        // Get the grid component
        m_Grid = m_Tunnel->GetComponent<GridComponent>();
        // Get sprite component
        m_Sprite = GetComponent<SpriteComponent>();
        // Get attack component
        m_Attack = GetOwner()->GetChildCount() > 0 ?
            GetOwner()->GetChildById(0)->GetComponent<AttackComponent>() : nullptr;

        // Snap to current tile center
        auto& transform { GetTransform() };
        transform.SetLocalPos(GetTileCenter(transform.GetLocalPos()));
    }

    void Player::Update(EngineCtx& ctx)
    {
        if (m_Tunnel == nullptr || m_Grid == nullptr)
            return;

        if (m_IsDead)
            return;

        HandleMovement(ctx.deltaTime);
        HandleAnimations();
    }

    void Player::OnDestroy(EngineCtx& ctx)
    {
        RemoveInput(ctx.inputManager);
    }

    void Player::OnOverlap(ICollider* other)
    {
        // If got hit by another attack
        if (auto* comp { dynamic_cast<Component*>(other) };
            comp != nullptr)
        {
            if (auto* attack { comp->GetComponent<AttackComponent>() };
                attack != nullptr && attack != m_Attack)
            {
                if (attack->CanHitPlayer())
                {
                    Die();

                    return;
                }
            }

            if (comp->HasComponent<Enemy>())
            {
                Die();

                return;
            }
        }
    }

    void Player::OnOverlapEnd(ICollider*)
    {

    }

    glm::vec2 Player::ClampToGrid(glm::vec2 pos) const
    {
        if (m_Grid == nullptr)
            return pos;

        const glm::vec2 gridOrigin { m_Grid->GetTransform().GetWorldPos() };
        const float tileSize { m_Grid->GetTileSize() };
        const float halfTile { tileSize * 0.5f };

        const int rows { static_cast<int>(m_Grid->GetTiles().Rows()) };
        const int cols { static_cast<int>(m_Grid->GetTiles().Cols()) };

        const float minX { gridOrigin.x + halfTile };
        const float maxX { gridOrigin.x + cols * tileSize - halfTile };
        const float minY { gridOrigin.y + halfTile };
        const float maxY { gridOrigin.y + rows * tileSize - halfTile };

        return glm::vec2 {
            pos.x < minX ? minX : (pos.x > maxX ? maxX : pos.x),
            pos.y < minY ? minY : (pos.y > maxY ? maxY : pos.y)
        };
    }

    void Player::HandleMovement(float deltaTime)
    {
        // Skip if can't move
        if (m_CurrentState != State::Idle && m_CurrentState != State::Moving &&
            m_CurrentState != State::Digging)
            return;

        // Skip if no input was present this frame
        if (glm::length(m_InputDir) < 0.01f)
        {
            m_CurrentState = State::Idle;

            return;
        }

        // Switch moving/digging state
        if (m_Tunnel->IsPlayerDigging(this))
            m_CurrentState = State::Digging;
        else
            m_CurrentState = State::Moving;

        // Consume input
        const auto inputDir { GetDir(m_InputDir) };
        m_InputDir = glm::vec2 { 0.0f, 0.0f };

        // Get transform and position
        auto& transform { GetTransform() };
        const auto oldPos { transform.GetLocalPos() };
        const auto displacement { inputDir * m_MoveSpeed * deltaTime };

        // Set initial state
        if (glm::length(m_LastDir) < 0.01f)
        {
            m_LastDir = inputDir;

            return;
        }

        // If going along m_MoveDir or opposite
        if (const auto dot { glm::dot(inputDir, m_LastDir) };
            dot > 0.9f || dot < -0.9f)
        {
            const auto newPos { ClampToGrid(oldPos + displacement) };

            transform.SetLocalPos(newPos);

            m_LastDir = inputDir;

            return;
        }

        // If going perpendicular (turning)

        const auto newPos { oldPos + (m_LastDir * m_MoveSpeed * deltaTime) };

        const auto center { GetTileCenter(newPos) };

        const float oldProj { glm::dot(oldPos, m_LastDir) };
        const float newProj { glm::dot(newPos, m_LastDir) };
        const float centerProj { glm::dot(center, m_LastDir) };

        const bool crossedCenter { centerProj >= oldProj && centerProj <= newProj };

        if (!crossedCenter)
        {
            transform.SetLocalPos(ClampToGrid(newPos));

            return;
        }

        const float distFromCenter { glm::length(displacement) -
            glm::distance(oldPos, center) };

        // After a corner, advance in the new direction (inputDir) from the center.
        // Clamp the resulting position to the grid so the first step of the new
        // direction also cannot escape.
        transform.SetLocalPos(ClampToGrid(center + (inputDir * distFromCenter)));

        m_LastDir = inputDir;
    }

    void Player::HandleAnimations()
    {
        static const std::array<std::string, 4> idleNames {
            "idle_up", "idle_right", "idle_down", "idle_left"
        };

        static const std::array<std::string, 4> moveNames {
            "move_up", "move_right", "move_down", "move_left"
        };

        static const std::array<std::string, 4> digNames {
            "dig_up", "dig_right", "dig_down", "dig_left"
        };

        switch (m_CurrentState)
        {
            case State::Idle:
                m_Sprite->SetAnimationIfChanged(idleNames[GetDirInt(m_LastDir)]);
                break;
            case State::Moving:
                m_Sprite->SetAnimationIfChanged(moveNames[GetDirInt(m_LastDir)]);
                break;
            case State::Digging:
                m_Sprite->SetAnimationIfChanged(digNames[GetDirInt(m_LastDir)]);
                break;
        }
    }

    void Player::SetMoveDir(glm::vec2 moveDir)
    {
        if (glm::length(m_InputDir) > 0.01f)
            return;

        m_InputDir = moveDir;
    }

    void Player::SetupInput(InputManager* input)
    {
        // Set up controller commands
        input->AddControllerCommand(m_PlayerId, ControllerButton::DPadDown,
            KeyState::Pressed, std::make_unique<PlayerMoveCommand>(this, glm::vec2 { 0.0f, 1.0f }));

        input->AddControllerCommand(m_PlayerId, ControllerButton::DPadUp,
            KeyState::Pressed, std::make_unique<PlayerMoveCommand>(this, glm::vec2 { 0.0f, -1.0f }));

        input->AddControllerCommand(m_PlayerId, ControllerButton::DPadRight,
            KeyState::Pressed, std::make_unique<PlayerMoveCommand>(this, glm::vec2 { 1.0f, 0.0f }));

        input->AddControllerCommand(m_PlayerId, ControllerButton::DPadLeft,
            KeyState::Pressed, std::make_unique<PlayerMoveCommand>(this, glm::vec2 { -1.0f, 0.0f }));

        input->AddControllerCommand(m_PlayerId, ControllerButton::ButtonX,
            KeyState::Down, std::make_unique<PlayerAttackCommand>(m_Attack, this, true));

        input->AddControllerCommand(m_PlayerId, ControllerButton::ButtonX,
            KeyState::Up, std::make_unique<PlayerAttackCommand>(m_Attack, this, false));

        // Set up keyboard commands
        if (m_PlayerId == 0)
        {
            input->AddKeyboardCommand(SDL_SCANCODE_S,
                KeyState::Pressed, std::make_unique<PlayerMoveCommand>(this, glm::vec2 { 0.0f, 1.0f }));

            input->AddKeyboardCommand(SDL_SCANCODE_W,
                KeyState::Pressed, std::make_unique<PlayerMoveCommand>(this, glm::vec2 { 0.0f, -1.0f }));

            input->AddKeyboardCommand(SDL_SCANCODE_D,
                KeyState::Pressed, std::make_unique<PlayerMoveCommand>(this, glm::vec2 { 1.0f, 0.0f }));

            input->AddKeyboardCommand(SDL_SCANCODE_A,
                KeyState::Pressed, std::make_unique<PlayerMoveCommand>(this, glm::vec2 { -1.0f, 0.0f }));

            input->AddKeyboardCommand(SDL_SCANCODE_F,
                KeyState::Down, std::make_unique<PlayerAttackCommand>(m_Attack, this, true));

            input->AddKeyboardCommand(SDL_SCANCODE_F,
                KeyState::Up, std::make_unique<PlayerAttackCommand>(m_Attack, this, false));
        }
        else if (m_PlayerId == 1)
        {
            input->AddKeyboardCommand(SDL_SCANCODE_DOWN,
                KeyState::Pressed, std::make_unique<PlayerMoveCommand>(this, glm::vec2 { 0.0f, 1.0f }));

            input->AddKeyboardCommand(SDL_SCANCODE_UP,
                KeyState::Pressed, std::make_unique<PlayerMoveCommand>(this, glm::vec2 { 0.0f, -1.0f }));

            input->AddKeyboardCommand(SDL_SCANCODE_RIGHT,
                KeyState::Pressed, std::make_unique<PlayerMoveCommand>(this, glm::vec2 { 1.0f, 0.0f }));

            input->AddKeyboardCommand(SDL_SCANCODE_LEFT,
                KeyState::Pressed, std::make_unique<PlayerMoveCommand>(this, glm::vec2 { -1.0f, 0.0f }));

            input->AddKeyboardCommand(SDL_SCANCODE_L,
                KeyState::Down, std::make_unique<PlayerAttackCommand>(m_Attack, this, true));

            input->AddKeyboardCommand(SDL_SCANCODE_L,
                KeyState::Up, std::make_unique<PlayerAttackCommand>(m_Attack, this, false));
        }
    }

    void Player::RemoveInput(InputManager* input)
    {
        // Remove controller commands
        input->RemoveControllerCommand(m_PlayerId, ControllerButton::DPadDown,
            KeyState::Pressed);

        input->RemoveControllerCommand(m_PlayerId, ControllerButton::DPadUp,
            KeyState::Pressed);

        input->RemoveControllerCommand(m_PlayerId, ControllerButton::DPadRight,
            KeyState::Pressed);

        input->RemoveControllerCommand(m_PlayerId, ControllerButton::DPadLeft,
            KeyState::Pressed);

        // Remove keyboard commands
        if (m_PlayerId == 0)
        {
            input->RemoveKeyboardCommand(SDL_SCANCODE_S, KeyState::Pressed);
            input->RemoveKeyboardCommand(SDL_SCANCODE_W, KeyState::Pressed);
            input->RemoveKeyboardCommand(SDL_SCANCODE_D, KeyState::Pressed);
            input->RemoveKeyboardCommand(SDL_SCANCODE_A, KeyState::Pressed);

            input->RemoveKeyboardCommand(SDL_SCANCODE_F, KeyState::Down);
            input->RemoveKeyboardCommand(SDL_SCANCODE_F, KeyState::Up);
        }
        else if (m_PlayerId == 1)
        {
            input->RemoveKeyboardCommand(SDL_SCANCODE_DOWN, KeyState::Pressed);
            input->RemoveKeyboardCommand(SDL_SCANCODE_UP, KeyState::Pressed);
            input->RemoveKeyboardCommand(SDL_SCANCODE_RIGHT, KeyState::Pressed);
            input->RemoveKeyboardCommand(SDL_SCANCODE_LEFT, KeyState::Pressed);

            input->RemoveKeyboardCommand(SDL_SCANCODE_L, KeyState::Down);
            input->RemoveKeyboardCommand(SDL_SCANCODE_L, KeyState::Up);
        }
    }

    glm::vec2 Player::GetTileCenter(glm::vec2 pos) const
    {
        if (m_Grid == nullptr)
            return pos;

        const auto coords { m_Grid->GetTileCoords(pos) };

        if (coords == INVALID_TILE_COORD)
            return pos;

        const auto gridOrigin { m_Grid->GetTransform().GetWorldPos() };
        const auto tileSize { m_Grid->GetTileSize() };

        return glm::vec2 {
            gridOrigin.x + (static_cast<float>(coords.second) + 0.5f) * tileSize,
            gridOrigin.y + (static_cast<float>(coords.first) + 0.5f) * tileSize
        };
    }

    glm::vec2 Player::GetDir(glm::vec2 inputDir) const
    {
        if (glm::length(inputDir) < 0.001f)
            return glm::vec2 { 0.0f, 0.0f };

        inputDir = glm::normalize(inputDir);

        const glm::vec2 dirs[4] =
        {
            {  0.0f, -1.0f }, // up
            {  1.0f,  0.0f }, // right
            {  0.0f,  1.0f }, // down
            { -1.0f,  0.0f }  // left
        };

        glm::vec2 bestDir = dirs[0];
        float bestDot = glm::dot(inputDir, dirs[0]);

        for (int i = 1; i < 4; i++)
        {
            float d = glm::dot(inputDir, dirs[i]);
            if (d > bestDot)
            {
                bestDot = d;
                bestDir = dirs[i];
            }
        }

        return bestDir;
    }

    int Player::GetDirInt(glm::vec2 dir) const
    {
        if (glm::length(dir) < 0.01f)
            return 0;

        dir = glm::normalize(dir);

        const glm::vec2 u { 0.0f, -1.0f };
        const glm::vec2 r { 1.0f,  0.0f };
        const glm::vec2 d { 0.0f,  1.0f };
        const glm::vec2 l { -1.0f,  0.0f };

        float bestDot = glm::dot(dir, u);
        int bestIndex = 0;

        float dotR = glm::dot(dir, r);
        if (dotR > bestDot) { bestDot = dotR; bestIndex = 1; }

        float dotD = glm::dot(dir, d);
        if (dotD > bestDot) { bestDot = dotD; bestIndex = 2; }

        float dotL = glm::dot(dir, l);
        if (dotL > bestDot) { bestDot = dotL; bestIndex = 3; }

        return bestIndex;
    }

    void Player::Reset()
    {
        // reset state
    }

    void Player::Die()
    {
        m_IsDead = true;

        m_Sprite->SetAnimation("die");

        m_EventManager->QueueEvent(Event { GameEvent::PlayerDied, m_PlayerId });
    }

    void PlayerMoveCommand::Execute()
    {
        m_Player->SetMoveDir(m_Direction);
    }

    void PlayerAttackCommand::Execute()
    {
        if (m_AttackComp == nullptr || m_Player == nullptr)
            return;

        if (m_Start)
            m_AttackComp->StartAttacking(m_Player->GetLastDirInt(), false);
        else
            m_AttackComp->StopAttacking();
    }
}
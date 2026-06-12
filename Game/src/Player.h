#pragma once

#include "GameObject.h"
#include "ICollisionReceiver.h"
#include "InputCommand.h"

namespace dae
{
    class TunnelComponent;
    class GridComponent;
    class SpriteComponent;
    class PlayerMoveCommand;
    class TestGraphCommand;
    class AttackComponent;

    class Player : public Component, public Registrar<Player>, public ICollisionReceiver
    {
        public:

        static void Register();

        explicit Player(GameObject* owner) : Component(owner) { };
        virtual ~Player() = default;

        void OnInit(EngineCtx& ctx) override;
        void Update(EngineCtx& ctx) override;
        void OnDestroy(EngineCtx& ctx) override;

        virtual void OnOverlap(ICollider* other) override;
        virtual void OnOverlapEnd(ICollider* other) override;

        int GetId() const { return m_PlayerId; }
        int GetLastDirInt() const { return GetDirInt(m_LastDir); }

        private:

        enum class State
        {
            Idle,
            Moving,
            Digging
        };

        friend class PlayerMoveCommand;
        friend class TestGraphCommand;

        // Helper to resolve movement
        void HandleMovement(float deltaTime);
        // Helper to resolve animations
        void HandleAnimations();

        // Called by PlayerMoveCommand
        void SetMoveDir(glm::vec2 moveDir);
        // Helper to set all the input commands
        void SetupInput(InputManager* input);
        // Helper to remove all the input commands
        void RemoveInput(InputManager* input);

        // Helper to get the center of a tile
        glm::vec2 GetTileCenter(glm::vec2 pos) const;
        // Helper to clamp a world-space position to the grid bounds
        glm::vec2 ClampToGrid(glm::vec2 pos) const;
        // Helper to get constrained direction (only up, down, left and right, no diagonals)
        glm::vec2 GetDir(glm::vec2 inputDir) const;
        // Helper to get encoded direction (0 = up, 1 = right, 2 = down, 3 = left)
        int GetDirInt(glm::vec2 dir)  const;

        // --- Lifetime helpers ---
        void Reset();
        void Die();

        // Params
        int m_PlayerId { };
        float m_MoveSpeed { };

        // State machine
        State m_CurrentState { State::Idle };

        // Movement state
        glm::vec2 m_InputDir { 0.0f, 0.0f };
        glm::vec2 m_LastDir { 0.0f, 0.0f };

        // Cached components
        TunnelComponent* m_Tunnel { };
        GridComponent* m_Grid { };
        SpriteComponent* m_Sprite { };
        AttackComponent* m_Attack { };

        EventManager* m_EventManager { };

        // Other
        bool m_IsDead { };
    };

    class PlayerMoveCommand final : public InputCommand
    {
        public:

        explicit PlayerMoveCommand(Player* player, glm::vec2 direction) :
            m_Player(player), m_Direction(direction)
        { };

        ~PlayerMoveCommand() override = default;

        void Execute() override;

        private:

        Player* m_Player { };
        glm::vec2  m_Direction { };
    };

    class PlayerAttackCommand final : public InputCommand
    {
        public:

        explicit PlayerAttackCommand(AttackComponent* attackComp, Player* player, bool start) :
            m_AttackComp(attackComp), m_Player(player), m_Start(start)
        { };

        void Execute() override;

        ~PlayerAttackCommand() override = default;

        private:

        Player* m_Player { };
        AttackComponent* m_AttackComp { };
        bool m_Start { };
    };
}
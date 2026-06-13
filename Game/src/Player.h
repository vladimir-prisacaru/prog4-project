#pragma once

#include "GameObject.h"
#include "ICollisionReceiver.h"
#include "InputCommand.h"
#include "DirHelpers.h"

namespace dae
{
    class TunnelComponent;
    class GridComponent;
    class SpriteComponent;
    class BoxCollider;
    class PlayerMoveCommand;
    class AttackComponent;
    class PlayerAttackCommand;
    class ServiceLocator;

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

        // Resets state fully and returns player to initial position
        void Reset();

        private:

        enum class State
        {
            Idle,
            Moving,
            Digging,
            Attacking,
            Dying
        };

        friend class PlayerMoveCommand;
        friend class PlayerAttackCommand;

        // Helper to resolve movement
        void HandleMovement(float deltaTime);
        // Helper to resolve animations
        void HandleAnimations();
        // Helper to resolve sounds based on current state
        void HandleSounds();
        // Helper to resolve death animation and deactivation
        void HandleDeath();

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

        // --- Lifetime helpers ---
        void Die(int killerId = -1);

        // Params
        int m_PlayerId { };
        float m_MoveSpeed { };

        // State machine
        State m_CurrentState { State::Idle };

        // Movement state
        glm::vec2 m_InputDir { 0.0f, 0.0f };
        glm::vec2 m_LastDir { 0.0f, 0.0f };

        // Initial spawn position (set in OnInit, used by Reset)
        glm::vec2 m_InitialPos { };

        // Cached components
        TunnelComponent* m_Tunnel { };
        GridComponent* m_Grid { };
        SpriteComponent* m_Sprite { };
        AttackComponent* m_Attack { };
        BoxCollider* m_BoxCollider { };

        // Cached managers (needed by Reset)
        InputManager* m_InputManager { };
        EventManager* m_EventManager { };
        ServiceLocator* m_Services { };
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
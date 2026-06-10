#pragma once

#include "GameObject.h"
#include "ICollisionReceiver.h"
#include "InputCommand.h"

namespace dae
{
    class TunnelComponent;
    class GridComponent;
    class PlayerMoveCommand;

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

        int GetId() { return m_PlayerId; }

        private:

        friend class PlayerMoveCommand;

        // Helper to resolve movement
        void HandleMovement(float deltaTime);

        // Called by PlayerMoveCommand
        void SetMoveDir(glm::vec2 moveDir);
        // Helper to set all the input commands
        void SetupInput(InputManager* input);
        // Helper to remove all the input commands
        void RemoveInput(InputManager* input);

        // Helper to get the center of a tile
        glm::vec2 GetTileCenter(glm::vec2 pos);
        // Helper to get constrained direction (only up, down, left and right, no diagonals)
        glm::vec2 GetDir(glm::vec2 inputDir);

        // Params
        int m_PlayerId { };
        float m_MoveSpeed { };

        // Movement state
        glm::vec2 m_InputDir { 0.0f, 0.0f };
        glm::vec2 m_LastDir { 0.0f, 0.0f };

        TunnelComponent* m_Tunnel { };
        GridComponent* m_Grid { };
    };

    class PlayerMoveCommand final : public InputCommand
    {
        public:

        explicit PlayerMoveCommand(Player* player, glm::vec2 direction) :
            m_Player(player), m_Direction(direction) { };

        ~PlayerMoveCommand() override = default;

        void Execute() override;

        private:

        Player* m_Player { };
        glm::vec2 m_Direction { };
    };
}
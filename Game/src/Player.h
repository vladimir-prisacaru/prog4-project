#pragma once

#include "GameObject.h"
#include "ICollisionReceiver.h"
#include "InputCommand.h"

namespace dae
{
    class TunnelComponent;
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

        // Called by PlayerMoveCommand
        void SetMoveDir(glm::vec2 moveDir);

        // Params
        int m_PlayerId { };
        float m_MoveSpeed { };

        // Other
        glm::vec2 m_InputMoveDir { 0.0f, 0.0f };
        TunnelComponent* m_Tunnel { };
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
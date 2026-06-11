#pragma once

#include "GameObject.h"
#include "ICollisionReceiver.h"
#include "TunnelComponent.h"

namespace dae
{
    class Player;

    class Enemy final : public Component, public Registrar<Enemy>, public ICollisionReceiver
    {
        public:

        static void Register();

        explicit Enemy(GameObject* owner) : Component(owner) { };
        ~Enemy() override = default;

        void OnInit(EngineCtx& ctx) override;
        void Update(EngineCtx& ctx) override;
        void OnDestroy(EngineCtx& ctx) override;

        void OnOverlap(ICollider* other) override;
        void OnOverlapEnd(ICollider* other) override;

        private:

        enum class State
        {
            Idle,
            Wander,
            Seek,
            Attack
        };

        // --- Path helpers ---
        bool PathfindToPlayer();
        bool FindWanderPath();

        // --- Movement helpers ---
        void HandleMovement(float deltaTime);

        // --- Params ---

        // How often the enemy pathfinds to the player (in seconds)
        float m_PathfindFrequency { };
        // How fast does the enemy move
        float m_MoveSpeed { };

        // --- Internal ---

        State m_CurrentState { State::Wander };

        // All active players
        std::vector<Player*> m_Players;
        // Currently targeted player
        Player* m_TargetPlayer { };
        // Active tunel component
        TunnelComponent* m_Tunnel { };
        // Graph of the tunnel component
        Graph* m_Graph { };
        // Currently followed/wandered path
        Path m_CurrentPath { };
        // Cached path to wander, will be moved to m_CurrentPath after enemy reaches its node 0
        Path m_CachedWanderPath { };
        // Id of the next node to reach in m_CurrentPath
        int m_NextPathNodeId { };
        // If the wandering has reached the end and is going in reverse through m_CurrentPath
        bool m_IsWanderingReversed { };
        // Acc time since last pathfind
        float m_PathfindTimer { };
    };
}
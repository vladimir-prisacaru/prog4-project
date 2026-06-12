#pragma once

#include "GameObject.h"
#include "ICollisionReceiver.h"
#include "TunnelComponent.h"
#include "DirHelpers.h"

namespace dae
{
    class Player;
    class SpriteComponent;
    class AttackComponent;

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
            Attack,
            Dying,
            Undying
        };

        // --- Path helpers ---
        bool PathfindToPlayer();
        bool FindWanderPath();

        // --- State ---
        void HandleMovement(float deltaTime);
        void HandleAttack();
        void HandleAnimations();
        void HandleUndeath();
        void HandleDeath();

        // --- Lifetime helpers ---
        void Die();

        // Checks if attacking is possible
        bool TryAttack();

        // --- Params ---

        // How often the enemy pathfinds to the player (in seconds)
        float m_PathfindFrequency { };
        // How fast does the enemy move
        float m_MoveSpeed { };
        // Supported attack directions within threshold
        std::vector<glm::vec2> m_AttackDirs { };
        // Supported attack angle threshold
        float m_AttackAngleThreshold { };
        // Max attack range
        float m_AttackRange { };

        // --- Internal ---

        State m_CurrentState { State::Idle };

        // All active players
        std::vector<Player*> m_Players;
        // Currently targeted player
        Player* m_TargetPlayer { };
        // Active tunnel component
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
        // Used to check if the enemy is currently being attacked
        AttackComponent* m_CurrentAttacker { };

        // Last movement direction (used for animations)
        glm::vec2 m_LastDir { 1.0f, 0.0f };

        // Cached
        SpriteComponent* m_Sprite { };
        AttackComponent* m_Attack { };
        Scene* m_Scene { };
        EventManager* m_EventManager { };
    };
}
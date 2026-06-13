#pragma once

#include "GameObject.h"
#include "ICollisionReceiver.h"

#include <functional>

namespace dae
{
    class SpriteComponent;
    class BoxCollider;
    class Physics;
    class ServiceLocator;

    class AttackComponent : public Component, public Registrar<AttackComponent>, public ICollisionReceiver
    {
        public:

        static void Register();

        explicit AttackComponent(GameObject* owner) : Component(owner) { };
        ~AttackComponent() override = default;

        void OnInit(EngineCtx& ctx) override;
        void Update(EngineCtx& ctx) override;

        // autoStop: stop automatically when the animation finishes
        // onHit: optional callback fired the first time this attack overlaps something
        void StartAttacking(int dir, bool autoStop = true,
            std::function<void(ICollider*)> onHit = nullptr);
        void StopAttacking();

        // Pause/resume: freezes the animation and skips autoStop checks,
        // but keeps the collider active so the overlap persists
        void PauseAttacking();
        void ResumeAttacking();

        bool IsAttacking() const { return m_IsAttacking; }
        bool IsPaused() const { return m_IsPaused; }
        bool IsFriendly() const { return m_IsFriendly; }
        bool CanHitPlayer() const { return m_CanHitPlayer; }
        bool IsFinished() const;

        // ICollisionReceiver
        void OnOverlap(ICollider* other) override;
        void OnOverlapEnd(ICollider* other) override;

        private:

        bool m_IsFriendly { };
        bool m_CanHitPlayer { };

        SpriteComponent* m_Sprite { };
        BoxCollider* m_BoxCollider { };
        Physics* m_Physics { };
        ServiceLocator* m_Services { };

        std::string m_CurrentAnimName { };
        int m_CurrentDir { -1 };
        bool m_IsAttacking { };
        bool m_IsPaused { };
        bool m_AutoStops { };

        std::function<void(ICollider*)> m_OnHitCallback { };
    };
}
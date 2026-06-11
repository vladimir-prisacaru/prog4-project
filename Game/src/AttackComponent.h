#pragma once

#include "GameObject.h"

namespace dae
{
    class SpriteComponent;
    class BoxCollider;

    class AttackComponent : public Component, public Registrar<AttackComponent>
    {
        public:

        static void Register();

        explicit AttackComponent(GameObject* owner) : Component(owner) { };
        ~AttackComponent() override = default;

        void OnInit(EngineCtx& ctx) override;
        void Update(EngineCtx& ctx) override;

        void StartAttacking(int dir, bool autoStop = true);
        void StopAttacking();

        bool IsAttacking() const { return m_IsAttacking; }
        bool IsFriendly() const { return m_IsFriendly; }
        bool CanHitPlayer() const { return m_CanHitPlayer; }
        bool IsFinished() const;

        private:

        bool m_IsFriendly { };
        bool m_CanHitPlayer { };

        SpriteComponent* m_Sprite { };
        BoxCollider* m_BoxCollider { };

        std::string m_CurrentAnimName { };
        bool m_IsAttacking { };
        bool m_AutoStops { };
    };
}
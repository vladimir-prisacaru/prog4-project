#include "AttackComponent.h"
#include "SpriteComponent.h"
#include "BoxCollider.h"

#include <array>

namespace dae
{
    void AttackComponent::Register()
    {
        RegisterComponent<AttackComponent>("attack_component");

        RegisterParameter("is_friendly", &AttackComponent::m_IsFriendly);
        RegisterParameter("can_hit_player", &AttackComponent::m_CanHitPlayer);
    }

    void AttackComponent::OnInit(EngineCtx& ctx)
    {
        m_Sprite = GetComponent<SpriteComponent>();
        m_BoxCollider = GetComponent<BoxCollider>();

        if (m_Sprite != nullptr)
            m_Sprite->SetAnimation("idle");

        if (m_BoxCollider != nullptr)
            m_BoxCollider->SetEnabled(false);
    }

    void AttackComponent::Update(EngineCtx& ctx)
    {
        if (m_Sprite == nullptr || m_BoxCollider == nullptr)
            return;

        if (!m_IsAttacking)
            return;

        if (m_AutoStops && m_Sprite->IsAnimationFinished("attack"))
        {
            StopAttacking();

            return;
        }

        Frame current { };

        if (!m_Sprite->GetCurrentFrame(current))
        {
            StopAttacking();

            return;
        }

        const float scale { m_Sprite->GetScale() };

        const glm::vec2 pivot { current.pivot };
        const float width { current.width * scale };
        const float height { current.height * scale };

        const glm::vec2 extents {
            width * 0.5f,
            height * 0.5f
        };

        const glm::vec2 center {
            (0.5f - pivot.x) * width,
            (0.5f - pivot.y) * height
        };

        m_BoxCollider->SetCenter(center);
        m_BoxCollider->SetExtents(extents);
    }

    void AttackComponent::StartAttacking(int dir, bool autoStop)
    {
        if (m_Sprite == nullptr || m_BoxCollider == nullptr)
            return;

        if (dir < 0 || dir > 3)
            return;

        static const std::array<std::string, 4> attackNames {
            "attack_up", "attack_right", "attack_down", "attack_left"
        };

        if (m_IsAttacking)
            return;

        m_BoxCollider->SetEnabled(true);
        m_CurrentAnimName = attackNames[dir];
        m_Sprite->SetAnimation(m_CurrentAnimName);
        m_AutoStops = autoStop;
    }

    void AttackComponent::StopAttacking()
    {
        if (!m_IsAttacking)
            return;

        m_BoxCollider->SetEnabled(false);
        m_Sprite->SetAnimation("idle");
    }

    bool AttackComponent::IsFinished() const
    {
        return m_Sprite->IsAnimationFinished("attack");
    }
}
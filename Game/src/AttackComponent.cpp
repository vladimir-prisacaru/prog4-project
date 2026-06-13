#include "AttackComponent.h"
#include "SpriteComponent.h"
#include "BoxCollider.h"
#include "GridCollider.h"
#include "Physics.h"
#include "DirHelpers.h"
#include "Enemy.h"

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
        m_Physics = ctx.physics;

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

        // While paused, skip animation advancement and autoStop checks
        if (!m_IsPaused && m_AutoStops && m_Sprite->IsAnimationFinished(m_CurrentAnimName))
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

        const float width { current.width * scale };
        const float height { current.height * scale };

        const glm::vec2 extents {
            width * 0.5f,
            height * 0.5f
        };

        const glm::vec2 center {
            (0.5f - current.pivot.x) * width,
            (0.5f - current.pivot.y) * height
        };

        // Raycast against the grid to find how far the attack can actually reach
        // before hitting a solid tile. If it hits, stop the attack completely.
        if (m_Physics != nullptr && m_CurrentDir >= 0 && m_CurrentDir <= 3)
        {
            const glm::vec2 dir { GetDirVec(m_CurrentDir) };
            const bool isVertical { m_CurrentDir == DIR_U || m_CurrentDir == DIR_D };
            const float maxDist { isVertical ? extents.y : extents.x };

            const Ray ray { GetTransform().GetWorldPos(), dir };
            const RaycastHit hit { m_Physics->Raycast<GridCollider>(ray, maxDist) };

            if (hit.hit)
            {
                StopAttacking();

                return;
            }
        }

        m_BoxCollider->SetCenter(center);
        m_BoxCollider->SetExtents(extents);
    }

    void AttackComponent::StartAttacking(int dir, bool autoStop,
        std::function<void(ICollider*)> onHit)
    {
        if (m_Sprite == nullptr || m_BoxCollider == nullptr)
            return;

        if (dir < 0 || dir > 3)
            return;

        if (m_IsAttacking)
            return;

        static const std::array<std::string, 4> attackNames {
            "attack_up", "attack_right", "attack_down", "attack_left"
        };

        m_CurrentDir = dir;
        m_CurrentAnimName = attackNames[dir];
        m_OnHitCallback = std::move(onHit);
        m_AutoStops = autoStop;
        m_IsPaused = false;

        m_Sprite->SetAnimation(m_CurrentAnimName);
        m_BoxCollider->SetEnabled(true);
        m_IsAttacking = true;
    }

    void AttackComponent::StopAttacking()
    {
        if (!m_IsAttacking)
            return;

        m_IsAttacking = false;
        m_IsPaused = false;
        m_OnHitCallback = nullptr;
        m_CurrentDir = -1;

        m_BoxCollider->SetEnabled(false);

        if (m_Sprite != nullptr)
        {
            m_Sprite->SetPaused(false);
            m_Sprite->SetAnimation("idle");
        }
    }

    void AttackComponent::PauseAttacking()
    {
        if (!m_IsAttacking || m_IsPaused)
            return;

        m_IsPaused = true;

        if (m_Sprite != nullptr)
            m_Sprite->SetPaused(true);
    }

    void AttackComponent::ResumeAttacking()
    {
        if (!m_IsAttacking || !m_IsPaused)
            return;

        m_IsPaused = false;

        if (m_Sprite != nullptr)
            m_Sprite->SetPaused(false);
    }

    bool AttackComponent::IsFinished() const
    {
        if (m_Sprite == nullptr || m_CurrentAnimName.empty())
            return false;

        return m_Sprite->IsAnimationFinished(m_CurrentAnimName);
    }

    void AttackComponent::OnOverlap(ICollider* other)
    {
        if (!m_IsAttacking || m_OnHitCallback == nullptr)
            return;

        m_OnHitCallback(other);
    }

    void AttackComponent::OnOverlapEnd(ICollider* other)
    {
        if (!m_IsPaused)
            return;

        // Resume when the enemy that caused the pause leaves (died or moved away)
        if (auto* comp { dynamic_cast<Component*>(other) };
            comp != nullptr && comp->HasComponent<Enemy>())
        {
            ResumeAttacking();
        }
    }
}
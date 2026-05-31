#include "HealthComponent.h"
#include "EventManager.h"

namespace dae
{
    HealthComponent::HealthComponent(GameObject* owner)
        : Component(owner)
    { }

    void HealthComponent::OnInit(EngineCtx& ctx)
    {
        m_Lives = m_MaxLives;

        m_EventManager = ctx.eventManager;
    }

    void HealthComponent::TakeDamage()
    {
        if (m_Lives <= 0) return;

        --m_Lives;

        m_EventManager->NotifyObservers(
            Event { GameEvent::PlayerDied, m_PlayerIndex });
    }
}
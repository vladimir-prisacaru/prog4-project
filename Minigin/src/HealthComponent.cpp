#include "HealthComponent.h"
#include "EventManager.h"

namespace dae
{
    HealthComponent::HealthComponent(GameObject* owner)
        : Component(owner)
    { }

    void HealthComponent::Initialize()
    {
        m_Lives = m_MaxLives;
    }

    void HealthComponent::TakeDamage()
    {
        if (m_Lives <= 0) return;

        --m_Lives;

        EventManager::GetInstance().NotifyObservers(
            Event { GameEvent::PlayerDied, m_PlayerIndex });
    }
}
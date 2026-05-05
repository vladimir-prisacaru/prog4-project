#include "LivesDisplayComponent.h"
#include "EventManager.h"
#include "TextComponent.h"
#include "Event.h"

#include <string>

namespace dae
{
    LivesDisplayComponent::LivesDisplayComponent(GameObject* owner)
        : Component(owner)
    {
        EventManager::GetInstance().AddListener(GameEvent::PlayerDied, this);

        UpdateText();
    }

    LivesDisplayComponent::~LivesDisplayComponent()
    {
        EventManager::GetInstance().RemoveListener(GameEvent::PlayerDied, this);
    }

    void LivesDisplayComponent::Notify(const Event& event)
    {
        if (event.id == GameEvent::PlayerDied)
        {
            if (event.value1 != m_PlayerIndex) return;

            if (m_Lives > 0) --m_Lives;
            UpdateText();
        }
    }

    void LivesDisplayComponent::UpdateText() const
    {
        auto text = GetOwner()->GetComponent<TextComponent>();

        if (text)
            text->SetText("P" + std::to_string(m_PlayerIndex + 1)
                + " Lives: " + std::to_string(m_Lives));
    }
}
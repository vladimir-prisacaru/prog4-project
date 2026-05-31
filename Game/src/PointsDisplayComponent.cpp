#include "PointsDisplayComponent.h"
#include "EventManager.h"
#include "TextComponent.h"
#include "Event.h"

#include <string>

namespace dae
{
    void PointsDisplayComponent::OnInit(EngineCtx& ctx)
    {
        ctx.eventManager->AddListener(GameEvent::PointsGained, this);
    }

    void PointsDisplayComponent::OnDestroy(EngineCtx& ctx)
    {
        ctx.eventManager->RemoveListener(GameEvent::PointsGained, this);
    }

    void PointsDisplayComponent::Notify(const Event& event)
    {
        if (event.id == GameEvent::PointsGained && m_PlayerId == event.value2)
        {
            m_Score += event.value1;
            UpdateText();
        }
    }

    void PointsDisplayComponent::UpdateText() const
    {
        auto* text = GetOwner()->GetComponent<TextComponent>();
        if (text)
            text->SetText("Score: " + std::to_string(m_Score));
    }
}
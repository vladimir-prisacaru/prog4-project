#include "PlayerUI.h"
#include "TextComponent.h"

namespace dae
{
    void PlayerUI::Register()
    {
        RegisterComponent<PlayerUI>("player_ui");

        RegisterParameter("target_player_id", &PlayerUI::m_TargetPlayerId);
    }

    void PlayerUI::OnInit(EngineCtx& ctx)
    {
        ctx.eventManager->AddListener(GameEvent::ScoreUpdated, this);
        ctx.eventManager->AddListener(GameEvent::LivesUpdated, this);

        if (GetOwner()->GetChildCount() < 2)
            return;

        m_ScoreText = GetOwner()->GetChildById(0)->GetComponent<TextComponent>();
        m_LivesText = GetOwner()->GetChildById(1)->GetComponent<TextComponent>();
    }

    void PlayerUI::OnDestroy(EngineCtx & ctx)
    {
        ctx.eventManager->RemoveListener(GameEvent::ScoreUpdated, this);
        ctx.eventManager->RemoveListener(GameEvent::LivesUpdated, this);
    }

    void PlayerUI::Notify(const Event & event)
    {
        if (m_ScoreText == nullptr || m_LivesText == nullptr)
            return;

        if (event.value1 != m_TargetPlayerId)
            return;

        std::string playerStr = m_TargetPlayerId < 0 ? "" :
            std::format("P{} ", m_TargetPlayerId);

        switch (event.id)
        {
            case GameEvent::ScoreUpdated:
                m_ScoreText->SetText(std::format("{}Score: {}", playerStr, event.value2));
                break;
            case GameEvent::LivesUpdated:
                m_LivesText->SetText(std::format("{}Lives: {}", playerStr, event.value2));
                break;
        }
    }
}
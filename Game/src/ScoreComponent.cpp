#include "ScoreComponent.h"
#include "EventManager.h"

namespace dae
{
    void ScoreComponent::OnInit(EngineCtx& ctx)
    {
        m_EventManager = ctx.eventManager;
    }

    void ScoreComponent::AddPoints(int points)
    {
        const int prevScore = m_Score;
        m_Score += points;

        m_EventManager->NotifyObservers(
            Event { GameEvent::PointsGained, points, m_PlayerIndex });

        if (prevScore < k_AchievementThreshold &&
            m_Score >= k_AchievementThreshold)
        {
            m_EventManager->NotifyObservers(
                Event { GameEvent::ScoreThresholdReached, m_PlayerIndex });
        }
    }
}
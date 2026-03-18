#include "ScoreComponent.h"
#include "EventManager.h"

namespace dae
{
    ScoreComponent::ScoreComponent(GameObject* owner)
        : Component(owner)
    { }

    void ScoreComponent::AddPoints(int points)
    {
        const int prevScore = m_Score;
        m_Score += points;

        EventManager::GetInstance().NotifyObservers(
            Event { GameEvent::PointsGained, points, m_PlayerIndex });

        if (prevScore < k_AchievementThreshold &&
            m_Score >= k_AchievementThreshold)
        {
            EventManager::GetInstance().NotifyObservers(
                Event { GameEvent::ScoreThresholdReached, m_PlayerIndex });
        }
    }
}
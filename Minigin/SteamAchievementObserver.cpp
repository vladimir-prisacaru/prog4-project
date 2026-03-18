#include "SteamAchievementObserver.h"
#include "Event.h"

#include <stdexcept>

namespace dae
{
    SteamAchievementObserver::SteamAchievementObserver(GameObject* owner) : Component(owner)
    {
        EventManager::GetInstance().AddListener(
            GameEvent::ScoreThresholdReached, this);
    }

    SteamAchievementObserver::~SteamAchievementObserver()
    {
        EventManager::GetInstance().RemoveListener(GameEvent::ScoreThresholdReached, this);
    }

    void SteamAchievementObserver::Notify(const Event& event)
    {
        if (event.id == GameEvent::ScoreThresholdReached)
            UnlockAchievement("ACH_WIN_ONE_GAME");
    }

    void SteamAchievementObserver::UnlockAchievement([[maybe_unused]] const char* achievementId)
    {
        #if USE_STEAMWORKS

        if (!SteamUserStats()) throw std::runtime_error("");

        SteamUserStats()->SetAchievement(achievementId);
        SteamUserStats()->StoreStats();

        #endif
    }
}
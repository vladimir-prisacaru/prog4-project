#include "SteamAchievementObserver.h"
#include "Event.h"

#include <stdexcept>

namespace dae
{
    SteamAchievementObserver::SteamAchievementObserver(GameObject* owner) : Component(owner)
    {
        #if USE_STEAMWORKS

        if (!SteamUserStats())
        {
            assert(false && "No steam users stats!");

            return;
        }

        bool achievedAlready = false;

        bool achievementExists =
            SteamUserStats()->GetAchievement("ACH_WIN_ONE_GAME", &achievedAlready);

        if (achievementExists && !achievedAlready)
        {
            EventManager::GetInstance().AddListener(
                GameEvent::ScoreThresholdReached, this);
        }

        #endif
    }

    SteamAchievementObserver::~SteamAchievementObserver()
    {
        #if USE_STEAMWORKS

        EventManager::GetInstance().RemoveListener(GameEvent::ScoreThresholdReached, this);

        #endif
    }

    void SteamAchievementObserver::Notify(const Event& event)
    {
        if (event.id == GameEvent::ScoreThresholdReached)
            UnlockAchievement("ACH_WIN_ONE_GAME");
    }

    void SteamAchievementObserver::UnlockAchievement([[maybe_unused]] const char* achievementId)
    {
        #if USE_STEAMWORKS

        if (!SteamUserStats())
        {
            assert(false && "No steam users stats!");

            return;
        }

        SteamUserStats()->SetAchievement(achievementId);
        SteamUserStats()->StoreStats();

        #endif
    }
}
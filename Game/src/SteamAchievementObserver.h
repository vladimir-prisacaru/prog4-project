#pragma once

#include "IObserver.h"
#include "GameObject.h"
#include "EventManager.h"

#if USE_STEAMWORKS
#pragma warning (push)
#pragma warning (disable:4996)
#include <steam_api.h>
#pragma warning (pop)
#endif

namespace dae
{
    class SteamAchievementObserver final : public Component, public IObserver
    {
        public:

        explicit SteamAchievementObserver(GameObject* owner);
        virtual ~SteamAchievementObserver();

        void Notify(const Event& event) override;

        private:

        void UnlockAchievement(const char* achievementId);
    };
}
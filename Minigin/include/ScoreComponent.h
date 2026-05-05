#pragma once

#include "GameObject.h"

namespace dae
{
    class ScoreComponent final : public Component
    {
        public:

        explicit ScoreComponent(GameObject* owner);
        ~ScoreComponent() override = default;

        void AddPoints(int points);

        int GetScore() const { return m_Score; }
        void SetPlayerIndex(int index) { m_PlayerIndex = index; }

        private:

        int m_Score { 0 };
        int m_PlayerIndex { 0 };

        static constexpr int k_AchievementThreshold { 500 };
    };
}
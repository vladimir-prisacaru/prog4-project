#pragma once

#include "GameObject.h"

namespace dae
{
    class HealthComponent final : public Component
    {
        public:

        explicit HealthComponent(GameObject* owner);
        ~HealthComponent() override = default;

        void Initialize() override;

        void TakeDamage();

        int  GetLives() const { return m_Lives; }
        int  GetPlayerIndex() const { return m_PlayerIndex; }

        void SetMaxLives(int lives) { m_MaxLives = lives; m_Lives = lives; }
        void SetPlayerIndex(int index) { m_PlayerIndex = index; }

        private:

        int m_MaxLives { 3 };
        int m_Lives { 3 };
        int m_PlayerIndex { 0 };
    };
}
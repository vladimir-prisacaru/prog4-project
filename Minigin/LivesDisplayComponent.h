#pragma once

#include "GameObject.h"
#include "IObserver.h"

namespace dae
{
    class LivesDisplayComponent final : public Component, public IObserver
    {
        public:

        explicit LivesDisplayComponent(GameObject* owner);
        ~LivesDisplayComponent() override;

        void Notify(const Event& event) override;

        void SetPlayerIndex(int index) { m_PlayerIndex = index; }
        void SetMaxLives(int lives) { m_Lives = lives; UpdateText(); }

        private:

        int m_PlayerIndex { 0 };
        int m_Lives { 3 };

        void UpdateText() const;
    };
}
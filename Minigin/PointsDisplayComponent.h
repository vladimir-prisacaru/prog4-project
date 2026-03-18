#pragma once

#include "GameObject.h"
#include "IObserver.h"

namespace dae
{
    class PointsDisplayComponent final : public Component, public IObserver
    {
        public:

        explicit PointsDisplayComponent(GameObject* owner);
        ~PointsDisplayComponent() override;

        void Notify(const Event& event) override;

        void SetPlayerId(int id) { m_PlayerId = id; };

        private:

        int m_PlayerId { 0 };
        int m_Score { 0 };

        void UpdateText() const;
    };
}
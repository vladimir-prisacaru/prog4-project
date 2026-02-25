#pragma once

#include "GameObject.h"
#include "TextComponent.h"

namespace dae
{
    class FPSCounter : public Component
    {
        public:

        FPSCounter(GameObject* parent) : Component(parent) { }
        virtual ~FPSCounter() = default;

        void Update(float deltaTime) override;

        private:

        float m_PrintInterval { 1.0f };

        float m_SumFPS { };
        int m_NumFrames { };

        float m_AccTime { };

        TextComponent* m_TextComponent { };
    };
}
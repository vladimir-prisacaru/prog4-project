#pragma once

#include "GameObject.h"
#include "TextComponent.h"

namespace dae
{
    class FPSCounter : public Component, public Registrar<FPSCounter>
    {
        public:

        static void Register();

        FPSCounter(GameObject* parent) : Component(parent) { }
        virtual ~FPSCounter() = default;

        void Update(EngineCtx& ctx) override;

        private:

        float m_PrintInterval { 1.0f };

        int m_NumFrames { };

        float m_AccTime { };

        TextComponent* m_TextComponent { };
    };
}
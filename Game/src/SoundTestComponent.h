#pragma once

#include <string>

#include "GameObject.h"

namespace dae
{
    class SoundTestComponent : public Component
    {
        public:

        explicit SoundTestComponent(GameObject* owner) : Component(owner) { };
        virtual ~SoundTestComponent() override = default;

        void Update(EngineCtx& ctx) override;

        private:

        std::string m_SoundName { "beep" };
        float m_Timer { 0 };
    };
}
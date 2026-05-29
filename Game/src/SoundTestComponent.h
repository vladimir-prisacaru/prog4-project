#pragma once

#include <string>

#include "GameObject.h"

namespace dae
{
    class SoundTestComponent : public Component
    {
        public:

        explicit SoundTestComponent(GameObject* owner);
        virtual ~SoundTestComponent() override = default;

        void Update(float) override;

        private:

        std::string m_SoundName { "beep" };
        float m_Timer { 0 };
    };
}
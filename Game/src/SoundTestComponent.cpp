#include "SoundTestComponent.h"
#include "ServiceLocator.h"
#include "SoundSystem.h"

namespace dae
{
    SoundTestComponent::SoundTestComponent(GameObject* owner) : Component(owner)
    {

    }

    void SoundTestComponent::Update(float deltaTime)
    {
        const float playSoundFrequency { 2.0f };

        if (m_Timer >= playSoundFrequency)
        {
            auto& soundSystem { ServiceLocator::GetSoundSystem() };

            soundSystem.Play(m_SoundName, 0.1f);

            m_Timer = 0.0f;
        }

        m_Timer += deltaTime;
    }
}
#include "SoundTestComponent.h"
#include "ServiceLocator.h"
#include "SoundSystem.h"

namespace dae
{
    void SoundTestComponent::Update(EngineCtx& ctx)
    {
        const float playSoundFrequency { 2.0f };

        if (m_Timer >= playSoundFrequency)
        {
            auto& soundSystem { ctx.services->GetSoundSystem() };

            soundSystem.Play(m_SoundName, 0.1f);

            m_Timer = 0.0f;
        }

        m_Timer += ctx.deltaTime;
    }
}
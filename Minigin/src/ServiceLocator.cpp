#include "ServiceLocator.h"
#include "SoundSystem.h"

#include <cassert>

namespace dae
{
    SoundSystem& ServiceLocator::GetSoundSystem()
    {
        assert(m_SoundSystem != nullptr &&
            "Attempted to get SoundSystem& from ServiceLocator, but m_SoundSystem is nullptr!");

        return *m_SoundSystem;
    }

    void ServiceLocator::RegisterSoundSystem(std::unique_ptr<SoundSystem>&& soundSystem)
    {
        m_SoundSystem = std::move(soundSystem);
    }

    void ServiceLocator::Cleanup()
    {
        m_SoundSystem.reset();
    }

    std::unique_ptr<SoundSystem> ServiceLocator::m_SoundSystem = nullptr;
}
#include "ServiceLocator.h"
#include "SoundSystem.h"

#include <cassert>

namespace dae
{
    ServiceLocator::ServiceLocator(const fs::path& dataPath) : m_DataPath(dataPath) { };
    ServiceLocator::~ServiceLocator() = default;

    SoundSystem& ServiceLocator::GetSoundSystem()
    {
        assert(m_SoundSystem != nullptr &&
            "Attempted to get SoundSystem& from ServiceLocator, but m_SoundSystem is nullptr!");

        return *m_SoundSystem;
    }

    void ServiceLocator::RegisterSoundSystem(std::unique_ptr<SoundSystem>&& soundSystem)
    {
        soundSystem->SetDataPath(m_DataPath);

        m_SoundSystem = std::move(soundSystem);
    }
}
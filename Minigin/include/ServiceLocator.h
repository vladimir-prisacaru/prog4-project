#pragma once

#include <memory>
#include <filesystem>
namespace fs = std::filesystem;

namespace dae
{
    class SoundSystem;

    class ServiceLocator final
    {
        public:

        explicit ServiceLocator(const fs::path& dataPath);

        ~ServiceLocator();
        ServiceLocator(const ServiceLocator& other) = delete;
        ServiceLocator(ServiceLocator&& other) = delete;
        ServiceLocator& operator=(const ServiceLocator& other) = delete;
        ServiceLocator& operator=(ServiceLocator&& other) = delete;

        SoundSystem& GetSoundSystem();
        void RegisterSoundSystem(std::unique_ptr<SoundSystem>&& soundSystem);

        private:

        fs::path m_DataPath;

        std::unique_ptr<SoundSystem> m_SoundSystem { };
    };
}
#pragma once

#include <memory>

namespace dae
{
    class SoundSystem;

    class ServiceLocator final
    {
        public:

        static SoundSystem& GetSoundSystem();
        static void RegisterSoundSystem(std::unique_ptr<SoundSystem>&& soundSystem);

        /* Call this for cleanup that needs to happen before main exits */
        static void Cleanup();

        private:

        static std::unique_ptr<SoundSystem> m_SoundSystem;
    };
}
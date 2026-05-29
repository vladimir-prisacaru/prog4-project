#pragma once

#include <string_view>

namespace dae
{
    class SoundSystem
    {
        public:

        virtual ~SoundSystem() = default;
        virtual void Play(std::string_view sound, float volume = 1.0f) = 0;
    };
}
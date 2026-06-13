#pragma once

#include <string_view>
#include <filesystem>
namespace fs = std::filesystem;

namespace dae
{
    class SoundSystem
    {
        public:

        virtual ~SoundSystem() = default;

        virtual void SetDataPath(const fs::path& dataPath) = 0;

        virtual void Play(std::string_view sound, float volume = 1.0f) = 0;
        virtual void PlayLooping(std::string_view sound, float volume = 1.0f) = 0;
        virtual void PlayIfNotPlaying(std::string_view sound, float volume = 1.0f) = 0;
        virtual void StopSound(std::string_view sound) = 0;
    };
}
#pragma once

#include "SoundSystem.h"

#include <filesystem>
namespace fs = std::filesystem;

namespace dae
{
    class SoundSystemSDL : public SoundSystem
    {
        public:

        SoundSystemSDL(const fs::path& clipsPath);
        virtual ~SoundSystemSDL();

        void SetDataPath(const fs::path& dataPath) override;
        void Play(std::string_view id, float volume = 1.0f) override;
        void PlayIfNotPlaying(std::string_view trackId, std::string_view audioName,
            float volume = 1.0f) override;
        void StopSound(std::string_view trackId) override;

        private:

        class Impl;
        std::unique_ptr<Impl> m_Impl { nullptr };
    };
}
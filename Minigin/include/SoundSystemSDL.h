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

        void Play(std::string_view id, float volume = 1.0f) override;

        private:

        class Impl;
        std::unique_ptr<Impl> m_Impl { nullptr };
    };
}
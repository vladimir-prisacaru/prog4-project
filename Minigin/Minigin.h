#pragma once

#include <string>
#include <functional>
#include <filesystem>
#include <chrono>

namespace dae
{
    class Minigin final
    {
        public:

        static constexpr float FIXED_TIMESTEP { 0.02f };
        static constexpr float MAX_FRAME_TIME { 0.25f };
        static constexpr float FRAME_TIME { 1.0f / 60.0f };

        explicit Minigin(const std::filesystem::path& dataPath);
        ~Minigin();

        Minigin(const Minigin& other) = delete;
        Minigin(Minigin&& other) = delete;
        Minigin& operator=(const Minigin& other) = delete;
        Minigin& operator=(Minigin&& other) = delete;

        void Run(const std::function<void()>& load);
        void RunOneFrame();

        private:

        bool m_Quit { };

        std::chrono::steady_clock::time_point m_LastUpdateTime {
            std::chrono::high_resolution_clock::now() };

        float m_FixedUpdateLag { };
    };
}
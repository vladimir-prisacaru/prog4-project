#pragma once

#include <string>
#include <functional>
#include <filesystem>
#include <chrono>

#include "EngineCtx.h"

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

        void Run(const std::function<void(EngineCtx& ctx)>& load);
        void RunOneFrame();

        private:

        std::unique_ptr<SceneManager> m_SceneManager { };
        std::unique_ptr<InputManager> m_InputManager { };
        std::unique_ptr<EventManager> m_EventManager { };
        std::unique_ptr<ResourceManager> m_ResourceManager { };
        std::unique_ptr<Renderer> m_Renderer { };
        std::unique_ptr<ServiceLocator> m_Services { };

        EngineCtx m_Context { };

        bool m_Quit { };

        std::chrono::steady_clock::time_point m_LastUpdateTime {
            std::chrono::high_resolution_clock::now() };

        float m_FixedUpdateLag { };

        #if USE_STEAMWORKS
        bool m_SteamInitialized { };
        #endif
    };
}
#pragma once

#include <vector>
#include <string>
#include <memory>
#include <string_view>
#include <filesystem>

#include "Scene.h"
namespace fs = std::filesystem;

namespace dae
{
    class Minigin;

    class SceneManager final
    {
        public:

        explicit SceneManager(const fs::path& scenesPath);

        ~SceneManager();
        SceneManager(const SceneManager& other) = delete;
        SceneManager(SceneManager&& other) = delete;
        SceneManager& operator=(const SceneManager& other) = delete;
        SceneManager& operator=(SceneManager&& other) = delete;

        Scene* CreateScene();
        Scene* LoadScene(const fs::path& file);
        void UnloadScene(Scene* scene);

        private:

        friend class Minigin;

        void Update(float deltaTime);
        void FixedUpdate(float deltaTime);

        void CleanupUnloadedScenes();

        fs::path m_ScenesPath { };

        std::vector<std::unique_ptr<Scene>> m_Scenes { };

        EngineCtx m_Ctx { };
    };
}
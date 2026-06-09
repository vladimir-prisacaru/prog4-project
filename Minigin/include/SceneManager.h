#pragma once

#include <vector>
#include <string>
#include <queue>
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

        /* Creates an empty scene */
        Scene* CreateScene();
        /* Schedules a scene to be loaded from a .xml file at the end of the frame */
        void LoadScene(const fs::path& file);
        /* Schedules a scene to be destroyed at the end of the frame */
        void UnloadScene(Scene* scene);

        /* Returns all objects from all loaded scenes that have a component of the given type */
        template <ComponentType T>
        std::vector<GameObject*> GetAllObjectsByType()
        {
            std::vector<GameObject*> result { };

            for (auto& scene : m_Scenes)
            {
                auto sceneResult { scene->GetAllObjectsByType<T>() };

                result.insert(result.end(),
                    std::make_move_iterator(sceneResult.begin()),
                    std::make_move_iterator(sceneResult.end()));
            }

            return result;
        }

        /* Returns the first object from all loaded scenes that has a component of the given type */
        template <ComponentType T>
        GameObject* GetFirstObjectByType()
        {
            for (auto& scene : m_Scenes)
            {
                if (GameObject * obj { scene->GetFirstObjectByType<T>() })
                    return obj;
            }

            return nullptr;
        }

        /* Returns all components from all loaded scenes of the given type */
        template <ComponentType T>
        std::vector<T*> GetAllComponentsByType()
        {
            std::vector<T*> result { };

            for (auto& scene : m_Scenes)
            {
                auto sceneResult { scene->GetAllComponentsByType<T>() };

                result.insert(result.end(),
                    std::make_move_iterator(sceneResult.begin()),
                    std::make_move_iterator(sceneResult.end()));
            }

            return result;
        }

        /* Returns the first component from all loaded scenes of the given type */
        template <ComponentType T>
        T* GetFirstComponentByType()
        {
            for (auto& scene : m_Scenes)
            {
                if (T * comp { scene->GetFirstComponentByType<T>() })
                    return comp;
            }

            return nullptr;
        }

        private:

        friend class Minigin;

        void Update(float deltaTime);
        void FixedUpdate(float deltaTime);

        void LoadAllScheduledScenes();
        Scene* LoadScheduledScene(const fs::path& path);

        void CleanupUnloadedScenes();

        fs::path m_ScenesPath { };

        std::queue<fs::path> m_ScheduledToLoad { };

        std::vector<std::unique_ptr<Scene>> m_Scenes { };

        EngineCtx m_Ctx { };
    };
}
#include "SceneManager.h"
#include "Minigin.h"

#include <cassert>

namespace dae
{
    SceneManager::SceneManager(const fs::path& scenesPath) : m_ScenesPath(scenesPath) { };
    SceneManager::~SceneManager() = default;

    Scene* SceneManager::CreateScene()
    {
        m_Scenes.emplace_back(new Scene());
        m_Scenes.back()->m_Ctx = m_Ctx;
        m_Scenes.back()->m_Ctx.scene = m_Scenes.back().get();
        return m_Scenes.back().get();
    }

    Scene* SceneManager::LoadScene(const fs::path& file)
    {
        tinyxml2::XMLDocument doc;

        fs::path fullPath { m_ScenesPath / file };

        if (doc.LoadFile(fullPath.string().c_str()) != tinyxml2::XML_SUCCESS)
        {
            //assert(false && "Failed to load scene file, recheck filepath or xml syntax");

            return nullptr;
        }

        tinyxml2::XMLElement* sceneElement { doc.FirstChildElement("scene") };

        if (!sceneElement)
        {
            assert(false && "Missing <scene> root element");

            return nullptr;
        }

        Scene* scene { CreateScene() };

        Scene::Parse(scene, sceneElement);

        return scene;
    }

    void SceneManager::UnloadScene(Scene* scene)
    {
        auto it = std::find_if(
            m_Scenes.begin(),
            m_Scenes.end(),
            [scene](const std::unique_ptr<Scene>& ptr)
            {
                return ptr.get() == scene;
            }
        );

        if (it == m_Scenes.end())
        {
            assert(false && "Tried to unload an invalid Scene!");

            return;
        }

        (*it)->m_MarkedForUnloading = true;
    }

    void SceneManager::Update(float deltaTime)
    {
        CleanupUnloadedScenes();

        for (auto& scene : m_Scenes)
        {
            scene->Update(deltaTime);
        }
    }

    void SceneManager::FixedUpdate(float deltaTime)
    {
        for (auto& scene : m_Scenes)
        {
            scene->FixedUpdate(deltaTime);
        }
    }

    void SceneManager::CleanupUnloadedScenes()
    {
        for (auto it { m_Scenes.begin() }; it != m_Scenes.end();)
        {
            if ((*it)->m_MarkedForUnloading)
            {
                (*it)->OnUnload();
                it = m_Scenes.erase(it);
            }
            else
                it++;
        }
    }
}
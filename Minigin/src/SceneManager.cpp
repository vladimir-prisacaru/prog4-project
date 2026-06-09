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

    void SceneManager::LoadScene(const fs::path& file)
    {
        fs::path fullPath { m_ScenesPath / file };

        if (!fs::exists(fullPath))
        {
            logError("Scene file at path '{}' does not exist!", fullPath.string());

            return;
        }

        m_ScheduledToLoad.push(fullPath);
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
        LoadAllScheduledScenes();

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

    void SceneManager::LoadAllScheduledScenes()
    {
        std::vector<Scene*> scenesToInit { };
        scenesToInit.reserve(m_ScheduledToLoad.size());

        while (!m_ScheduledToLoad.empty())
        {
            fs::path path { std::move(m_ScheduledToLoad.front()) };
            m_ScheduledToLoad.pop();

            if (Scene* scene { LoadScheduledScene(path) })
                scenesToInit.push_back(scene);
        }

        for (auto& scene : scenesToInit)
        {
            scene->Init();
        }
    }

    Scene* SceneManager::LoadScheduledScene(const fs::path& path)
    {
        tinyxml2::XMLDocument doc;

        if (doc.LoadFile(path.string().c_str()) != tinyxml2::XML_SUCCESS)
        {
            logError(".xml parse error in file '{}': {}", path.string(), doc.ErrorStr());

            return nullptr;
        }

        tinyxml2::XMLElement* sceneElement { doc.FirstChildElement("scene") };

        if (!sceneElement)
        {
            logError("Missing <scene> root element in scene file '{}'", path.string());

            return nullptr;
        }

        Scene* scene { CreateScene() };

        Scene::Parse(scene, sceneElement);

        return scene;
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
#pragma once

#include <memory>
#include <string>
#include <vector>

#include "GameObject.h"

namespace dae
{
    class Scene final
    {
        public:

        /* Instantiates a new GameObject */
        GameObject* Instantiate();
        /* Destroys the provided GameObject */
        void Destroy(GameObject* object);

        ~Scene();
        Scene(const Scene& other) = delete;
        Scene(Scene&& other) = delete;
        Scene& operator=(const Scene& other) = delete;
        Scene& operator=(Scene&& other) = delete;

        private:

        friend class SceneManager;

        static void Parse(Scene* scene, tinyxml2::XMLElement* sceneElement);

        explicit Scene() = default;

        void Update(float deltaTime);
        void FixedUpdate(float deltaTime);

        void OnUnload();

        void CleanupDestroyedObjects();

        bool m_MarkedForUnloading { };

        std::vector<std::unique_ptr<GameObject>> m_Objects { };

        EngineCtx m_Ctx { };
    };
}
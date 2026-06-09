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

        /* Returns all objects that have a component of the given type */
        template <ComponentType T>
        std::vector<GameObject*> GetAllObjectsByType()
        {
            std::vector<GameObject*> result { };

            for (auto& obj : m_Objects)
            {
                if (obj->HasComponent<T>())
                    result.push_back(obj);
            }

            return result;
        }

        /* Returns the first object that has a component of the given type */
        template <ComponentType T>
        GameObject* GetFirstObjectByType()
        {
            for (auto& obj : m_Objects)
            {
                if (obj->HasComponent<T>())
                    return obj.get();
            }

            return nullptr;
        }

        /* Returns all components of the given type */
        template <ComponentType T>
        std::vector<T*> GetAllComponentsByType()
        {
            std::vector<T*> result { };

            for (auto& obj : m_Objects)
            {
                if (T* comp { obj->GetComponent<T>() })
                    result.push_back(comp);
            }

            return result;
        }

        /* Returns the first component of the given type */
        template <ComponentType T>
        T* GetFirstComponentByType()
        {
            for (auto& obj : m_Objects)
            {
                if (T* comp { obj->GetComponent<T>() })
                    return comp;
            }

            return nullptr;
        }

        ~Scene();
        Scene(const Scene& other) = delete;
        Scene(Scene&& other) = delete;
        Scene& operator=(const Scene& other) = delete;
        Scene& operator=(Scene&& other) = delete;

        private:

        friend class SceneManager;

        static void Parse(Scene* scene, tinyxml2::XMLElement* sceneElement);

        explicit Scene() = default;

        void Init();
        void Update(float deltaTime);
        void FixedUpdate(float deltaTime);

        void OnUnload();

        void CleanupDestroyedObjects();

        bool m_MarkedForUnloading { };

        std::vector<std::unique_ptr<GameObject>> m_Objects { };

        EngineCtx m_Ctx { };
    };
}
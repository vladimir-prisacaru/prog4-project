#pragma once

#include "GameObject.h"

#include <memory>
#include <string>
#include <vector>

namespace dae
{
    class Scene final
    {
        public:

        void Add(std::unique_ptr<GameObject> object);
        void Remove(const GameObject& object);
        void RemoveAll();

        void Update(float deltaTime);
        void FixedUpdate(float deltaTime);
        void Render() const;

        void CleanupMarked();

        ~Scene() = default;
        Scene(const Scene& other) = delete;
        Scene(Scene&& other) = delete;
        Scene& operator=(const Scene& other) = delete;
        Scene& operator=(Scene&& other) = delete;

        private:

        friend class SceneManager;

        explicit Scene() = default;

        std::vector<std::unique_ptr<GameObject>> m_Objects { };
    };
}
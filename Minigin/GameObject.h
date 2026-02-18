#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <typeindex>
#include <memory>

#include "Transform.h"

namespace dae
{
    class Texture2D;
    class Component;

    class GameObject final
    {
        public:

        GameObject() = default;
        virtual ~GameObject();

        GameObject(const GameObject& other) = delete;
        GameObject(GameObject&& other) = delete;
        GameObject& operator=(const GameObject& other) = delete;
        GameObject& operator=(GameObject&& other) = delete;

        void Initialize();
        void Update(float deltaTime);
        void FixedUpdate(float deltaTime);
        void Render() const;

        glm::vec2 GetPosition();
        void SetPosition(float x, float y);

        /* Adds a component of type T to the object and returnsa non-owning pointer to it */
        template<typename T>
            requires std::is_base_of_v<Component, T>
        T* AddComponent()
        {
            if (HasComponent<T>())
            {
                assert(false && "object already has a component of the same type");

                return nullptr;
            }

            const std::type_index id { typeid(T) };

            m_ComponentMap[id] = std::make_unique<T>(this);

            return static_cast<T*>(m_ComponentMap[id].get());
        }

        /* Returns a component of the object based on type
           (returns nullptr if object has no component of type T) */
        template<typename T>
            requires std::is_base_of_v<Component, T>
        T* GetComponent()
        {
            auto it = m_ComponentMap.find(typeid(T));

            if (it == m_ComponentMap.end())
                return nullptr;

            return static_cast<T*>(it->second.get());
        }

        /* Removes a component from the object based on type */
        template<typename T>
            requires std::is_base_of_v<Component, T>
        void RemoveComponent()
        {
            auto it = m_ComponentMap.find(typeid(T));

            if (it == m_ComponentMap.end())
            {
                assert(false && "object does not have a component of this type");

                return;
            }

            m_ComponentMap.erase(it);
        }

        /* Checks if an object has a component of type T */
        template<typename T>
            requires std::is_base_of_v<Component, T>
        bool HasComponent()
        {
            return m_ComponentMap.find(typeid(T)) != m_ComponentMap.end();
        }

        private:

        Transform m_Transform { };

        std::unordered_map<std::type_index, std::unique_ptr<Component>> m_ComponentMap { };
    };

    class Component
    {
        public:

        Component(GameObject* parent) : m_Parent(parent) { }
        virtual ~Component() = default;

        /* Called when the component is created */
        virtual void Initialize() { }

        /* Called every frame */
        virtual void Update(float deltaTime) { std::ignore = deltaTime; }

        /* Called every fixed timestep */
        virtual void FixedUpdate(float deltaTime) { std::ignore = deltaTime; }

        /* Called on draw */
        virtual void Render() const { }

        protected:

        GameObject* m_Parent { };
    };
}
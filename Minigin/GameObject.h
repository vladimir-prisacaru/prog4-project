#pragma once

#include <glm/glm.hpp>

#include <vector>
#include <unordered_map>
#include <typeindex>
#include <memory>

namespace dae
{
    class Component;
    class Transform;
    class GameObject;

    template<typename T>
    concept ComponentType = std::is_base_of_v<Component, T>&&
        requires(GameObject* owner)
    {
        { std::make_unique<T>(owner) } -> std::same_as<std::unique_ptr<T>>;
    };



    class Component
    {
        public:

        Component(GameObject* owner) : m_Owner(owner) { }
        virtual ~Component() = default;

        Component(Component&) = delete;
        Component& operator=(Component&) = delete;
        Component(Component&&) = delete;
        Component& operator=(Component&&) = delete;

        /* Called when the component is created */
        virtual void Initialize() { }

        /* Called every frame */
        virtual void Update(float) { }

        /* Called every fixed timestep */
        virtual void FixedUpdate(float) { }

        /* Called on draw */
        virtual void Render() const { }

        /* Returns the owning game object of this component */
        GameObject* GetOwner() const;

        private:

        GameObject* m_Owner { };
    };



    class Transform final
    {
        public:

        Transform(GameObject* owner);

        /* Returns the position relative to the parent */
        glm::vec3 GetLocalPos() const;
        /* Sets the position relative to the parent */
        void SetLocalPos(float x, float y, float z = 0);
        /* Sets the position relative to the parent */
        void SetLocalPos(const glm::vec3& position);
        /* Returns the absolute position */
        glm::vec3 GetWorldPos() const;
        /* Sets the dirty flag to recalculate world position */
        void MarkWorldPosDirty();

        private:

        GameObject* m_Owner { };

        glm::vec3 m_LocalPosition { };

        mutable glm::vec3 m_WorldPosition { };
        mutable bool m_WorldPosDirty { true };
    };



    class GameObject final
    {
        public:

        GameObject() = default;
        ~GameObject() = default;

        GameObject(const GameObject& other) = delete;
        GameObject(GameObject&& other) = delete;
        GameObject& operator=(const GameObject& other) = delete;
        GameObject& operator=(GameObject&& other) = delete;

        /* Called when the object is created */
        void Initialize();

        /* Called every frame */
        void Update(float deltaTime);

        /* Called every fixed timestep */
        void FixedUpdate(float deltaTime);

        /* Called on draw */
        void Render() const;

        /* Reparents the parent of this object */
        Transform& GetTransform();

        /* Reparents the parent of this object */
        GameObject* GetParent() const;

        /* Reparents this object to a new game object */
        void SetParent(GameObject* parent, bool keepWorldPos = true);

        /* Returns the number of children of this object */
        unsigned int GetChildCount() const;

        /* Returns a child of this object by its index */
        GameObject* GetChildById(unsigned int id) const;

        /* Returns the array of children of this object */
        const std::vector<GameObject*>& GetChildren() const;

        /* Returns true if obj is a descendant of this object */
        bool IsChild(GameObject* obj) const;

        /* Returns true if this object is marked for deletion */
        bool IsMarkedForDeletion() const;

        /* Marks this object and all children for deletion */
        void MarkForDeletion();

        /* Removes children scheduled for deletion */
        void RemoveChildrenMarkedForDeletion();

        /* Adds a component of type T to the object and returns a non-owning pointer to it */
        template<typename T>
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
        template<ComponentType T>
        T* GetComponent()
        {
            auto it = m_ComponentMap.find(typeid(T));

            if (it == m_ComponentMap.end())
                return nullptr;

            return static_cast<T*>(it->second.get());
        }

        /* Removes a component from the object based on type */
        template<ComponentType T>
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
        template<ComponentType T>
        bool HasComponent()
        {
            return m_ComponentMap.find(typeid(T)) != m_ComponentMap.end();
        }

        private:

        Transform m_Transform { this };

        bool m_MarkedForDeletion { };

        std::unordered_map<std::type_index, std::unique_ptr<Component>> m_ComponentMap { };

        GameObject* m_Parent { };
        std::vector<GameObject*> m_Children { };
    };
}
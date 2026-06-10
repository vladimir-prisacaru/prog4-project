#pragma once

#include <glm/glm.hpp>
#include <SDL3/SDL.h>
#include <tinyxml2.h>

#include <vector>
#include <unordered_map>
#include <typeindex>
#include <memory>
#include <sstream>
#include <functional>
#include <type_traits>
#include <list>

#include "Array2d.h"

#include "EngineCtx.h"
#include "DebugUtils.h"

namespace dae
{
    class Component;
    class Transform;
    class GameObject;
    class Scene;



    // Static registrar, calls Derived::Register() during static initialization
    template <typename Derived>
    class Registrar
    {
        private:

        static inline const struct Reg
        {
            Reg()
            {
                Derived::Register();
            }
        } reg { };
    };



    // Any derived component must conform to this concept
    template<typename T>
    concept ComponentType =
        std::is_base_of_v<Component, T> &&
        std::is_base_of_v<Registrar<T>, T> &&
        requires(GameObject* owner)
    {
        { std::make_unique<T>(owner) } -> std::same_as<std::unique_ptr<T>>;
        { T::Register() } -> std::same_as<void>;
    };



    // Traits to determine if T is a vector, and extract element type
    template <typename T>
    struct is_vector : std::false_type { };

    template <typename T, typename Alloc>
    struct is_vector<std::vector<T, Alloc>> : std::true_type { };

    template <typename T>
    inline constexpr bool is_vector_v = is_vector<T>::value;

    template <typename T>
    struct vector_element { using type = void; };

    template <typename T, typename Alloc>
    struct vector_element<std::vector<T, Alloc>> { using type = T; };

    template <typename T>
    using vector_element_t = typename vector_element<T>::type;



    // Traits to determine if T is an Array2d, and extract element type
    template <typename T>
    struct is_array_2d : std::false_type { };

    template <typename T>
    struct is_array_2d<Array2d<T>> : std::true_type { };

    template <typename T>
    inline constexpr bool is_array_2d_v = is_array_2d<T>::value;

    template <typename T>
    struct array2d_element { using type = void; };

    template <typename T>
    struct array2d_element<Array2d<T>> { using type = T; };

    template <typename T>
    using array2d_element_t = typename array2d_element<T>::type;



    // Umbrella trait to determine if T is any container
    template <typename T>
    inline constexpr bool is_container_v = is_vector_v<T> || is_array_2d_v<T>;



    class Transform final
    {
        public:

        Transform(GameObject* owner);

        /* Returns the position relative to the parent */
        glm::vec2 GetLocalPos() const;
        /* Sets the position relative to the parent */
        void SetLocalPos(float x, float y);
        /* Sets the position relative to the parent */
        void SetLocalPos(const glm::vec2& position);
        /* Returns the absolute position */
        glm::vec2 GetWorldPos() const;
        /* Sets the dirty flag to recalculate world position */
        void MarkWorldPosDirty();
        /* Parses transform parameters from the XML element */
        void Parse(tinyxml2::XMLElement* element);

        private:

        GameObject* m_Owner { };

        glm::vec2 m_LocalPosition { };

        mutable glm::vec2 m_WorldPosition { };
        mutable bool m_WorldPosDirty { true };
    };



    class GameObject final
    {
        public:

        ~GameObject() = default;
        GameObject(const GameObject& other) = delete;
        GameObject(GameObject&& other) = delete;
        GameObject& operator=(const GameObject& other) = delete;
        GameObject& operator=(GameObject&& other) = delete;

        /* Returns the transform of this object */
        Transform& GetTransform();

        /* Returns the parent of this object */
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

        /* Adds a component of type T to the object and returns a non-owning pointer to it */
        template<ComponentType T>
        T* AddComponent()
        {
            if (HasComponent<T>())
            {
                assert(false && "object already has a component of the same type");

                return nullptr;
            }

            const std::type_index id { typeid(T) };

            m_ComponentMap[id] = std::make_unique<T>(this);

            T* component { static_cast<T*>(m_ComponentMap[id].get()) };

            component->OnInit(m_Ctx); // call OnInit

            return static_cast<T*>(m_ComponentMap[id].get());
        }

        /* Returns a component of the object based on type
           (returns nullptr if object has no component of type T) */
        template<ComponentType T>
        T* GetComponent() const
        {
            auto it = m_ComponentMap.find(typeid(T));

            if (it == m_ComponentMap.end())
                return nullptr;

            return static_cast<T*>(it->second.get());
        }

        /* Return a component of it is can be dynamic_cast to the type T
           (T can be any type, for example, an interface) */
        template<typename T>
        T* GetComponentOfType() const
        {
            for (auto& [id, comp] : m_ComponentMap)
            {
                T* component { dynamic_cast<T*>(comp.get()) };

                if (component == nullptr)
                    continue;

                return component;
            }

            return nullptr;
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

            it->second->OnDestroy(m_Ctx);

            m_ComponentMap.erase(it);
        }

        /* Checks if an object has a component of type T */
        template<ComponentType T>
        bool HasComponent()
        {
            return m_ComponentMap.find(typeid(T)) != m_ComponentMap.end();
        }

        private:

        friend class Scene;
        friend class Component;

        explicit GameObject() = default;

        /* Adds a component of type T to the object and returns a non-owning pointer to it (without calling OnInit) */
        template<ComponentType T>
        T* AddComponentNoInit()
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

        static void Parse(GameObject* object, tinyxml2::XMLElement* element,
            std::vector<tinyxml2::XMLElement*>& childElements);

        void OnInit(EngineCtx& ctx);
        void Update(EngineCtx& ctx);
        void FixedUpdate(EngineCtx& ctx);
        void OnDestroy(EngineCtx& ctx);

        void PropagateMarkedForDeletion();
        void RemoveChildrenMarkedForDeletion();

        Transform m_Transform { this };

        bool m_MarkedForDeletion { };

        std::unordered_map<std::type_index, std::unique_ptr<Component>> m_ComponentMap { };

        GameObject* m_Parent { };
        std::vector<GameObject*> m_Children { };

        EngineCtx m_Ctx { };
    };



    class Component
    {
        public:

        using ComponentFactory = std::unordered_map<std::string,
            std::function<Component* (GameObject* parent)>>;

        using ParameterRegistry = std::unordered_map<std::type_index,
            std::unordered_map<std::type_index, std::unordered_map<std::string,
            std::function<void(Component*, const std::string&)>>>>;

        Component(GameObject* owner) : m_Owner(owner) { }
        virtual ~Component() = default;

        Component(Component&) = delete;
        Component& operator=(Component&) = delete;
        Component(Component&&) = delete;
        Component& operator=(Component&&) = delete;

        /* Returns the map of component names and their respective creation functions */
        static ComponentFactory& GetComponentFactory();

        /* Sets the parameters of the given component with values parsed from an XMLElement */
        static void Parse(Component* component, tinyxml2::XMLElement* componentElement);

        /* Called when the component is created */
        virtual void OnInit(EngineCtx&) { }

        /* Called every frame */
        virtual void Update(EngineCtx&) { }

        /* Called every fixed timestep */
        virtual void FixedUpdate(EngineCtx&) { }

        /* Called before the object is destroyed */
        virtual void OnDestroy(EngineCtx&) { }

        /* Returns the owning GameObject of this component */
        GameObject* GetOwner() const;

        /* Same as GetOwner()->GetTransform() */
        Transform& GetTransform() const;

        /* Same as GetOwner()->GetComponent<T>() */
        template<ComponentType T>
        T* GetComponent() const
        {
            return m_Owner->GetComponent<T>();
        }

        /* Same as GetOwner()->GetComponentOfType<T>() */
        template<typename T>
        T* GetComponentOfType()
        {
            return m_Owner->GetComponentOfType<T>();
        }

        protected:

        /* Registers the component type in the component factory for parsing (use in a static registrar) */
        template<ComponentType CompT>
        static void RegisterComponent(const std::string& name)
        {
            auto& factory = GetFactory();

            if (factory.contains(name))
            {
                logError("Component type with name '{}' is already registered", name);

                return;
            }

            factory[name] = [](GameObject* obj) { return obj->AddComponentNoInit<CompT>(); };
        }

        /* Registers the parameters for parsing */
        template <typename T, ComponentType CompT>
        static void RegisterParameter(const std::string& key, T CompT::* member)
        {
            auto& registry = GetRegistry()[typeid(CompT)][typeid(T)];
            registry[key] = [key, member] (Component* instance, const std::string& valueStr)
            {
                std::istringstream iss(valueStr);
                T value { };

                if constexpr (is_container_v<T>)
                {
                    if (!ParseContainer(iss, value))
                    {
                        logError("Failed to parse <{}> container parameter from string: {}",
                            key, valueStr);

                        return;
                    }
                }
                else
                {
                    if (!ParseSingle(iss, value))
                    {
                        logError("Failed to parse <{}> single parameter from string: {}",
                            key, valueStr);

                        return;
                    }
                }

                static_cast<CompT*>(instance)->*member = value;
            };
        }

        private:

        /* Parses a single value */
        template <typename T>
        static bool ParseSingle(std::istringstream& iss, T& value)
        {
            if constexpr (std::is_same_v<T, glm::vec2>)
            {
                if (!(iss >> value.x >> value.y))
                {
                    logError("Failed to parse glm::vec2 value.");

                    return false;
                }

                return true;
            }
            else if constexpr (std::is_same_v<T, glm::vec3>)
            {
                if (!(iss >> value.x >> value.y >> value.z))
                {
                    logError("Failed to parse glm::vec3 value.");

                    return false;
                }

                return true;
            }
            else if constexpr (std::is_same_v<T, glm::vec4>)
            {
                if (!(iss >> value.x >> value.y >> value.z >> value.w))
                {
                    logError("Failed to parse glm::vec4 value.");

                    return false;
                }

                return true;
            }
            else if constexpr (std::is_same_v<T, SDL_Color>)
            {
                glm::vec4 val { };

                if (!(iss >> val.x >> val.y >> val.z >> val.w))
                {
                    logError("Failed to parse SDL_Color value.");

                    return false;
                }

                const glm::vec4 clamped { glm::clamp(val, 0.0f, 1.0f) };

                value = SDL_Color {
                    static_cast<Uint8>(clamped.r * 255.0f),
                    static_cast<Uint8>(clamped.g * 255.0f),
                    static_cast<Uint8>(clamped.b * 255.0f),
                    static_cast<Uint8>(clamped.a * 255.0f)
                };

                return true;
            }
            else if constexpr (std::is_same_v<T, std::string>)
            {
                value = std::string(
                    std::istreambuf_iterator<char>(iss),
                    std::istreambuf_iterator<char>()
                );

                if (value.empty())
                {
                    logError("Failed to parse std::string value.");

                    return false;
                }

                return true;
            }
            else if constexpr (std::is_same_v<T, bool>)
            {
                iss >> std::boolalpha;

                if (!(iss >> value))
                {
                    logError("Failed to parse bool value.");

                    return false;
                }

                iss >> std::noboolalpha;

                return true;
            }
            else
            {
                if (!(iss >> value))
                {
                    logError("Failed to parse value.");

                    return false;
                }

                return true;
            }
        }

        /* Parses a container of values */
        template <typename ContainerT>
        static bool ParseContainer(std::istringstream& iss, ContainerT& container)
        {
            if constexpr (is_vector_v<ContainerT>)
            {
                using ElemT = vector_element_t<ContainerT>;

                while (!iss.eof())
                {
                    ElemT elem {};

                    if (!ParseSingle(iss, elem))
                    {
                        logError("Failed to parse std::vector element value");

                        return false;
                    }

                    container.push_back(elem);
                }

                if (container.empty())
                {
                    logError("Failed to parse std::vector value");

                    return false;
                }

                return true;
            }
            else if constexpr (is_array_2d_v<ContainerT>)
            {
                int rows { };
                int cols { };

                if (!(iss >> rows >> cols))
                {
                    logError("Failed to parse rows/cols value of Array2d");

                    return false;
                }

                container.Resize(rows, cols);

                for (int r = 0; r < rows; r++)
                {
                    for (int c = 0; c < cols; c++)
                    {
                        if (!ParseSingle(iss, container(r, c)))
                        {
                            logError("Failed to parse cell [{}][{}] values of Array2d", r, c);

                            return false;
                        }
                    }
                }

                return true;
            }
            else
            {
                static_assert(!sizeof(ContainerT), "ParseContainer: unsupported container type");
                return false;
            }
        }

        /* Returns the map of component names and their respective creation functions */
        static ComponentFactory& GetFactory()
        {
            static ComponentFactory factory;
            return factory;
        }

        /* Returns the registry of all parameters of the component */
        static ParameterRegistry& GetRegistry()
        {
            static ParameterRegistry registry;
            return registry;
        }

        GameObject* m_Owner { };
    };
}
#include "GameObject.h"
#include "Scene.h"

#include <algorithm>

namespace dae
{
    // -----------------
    // --- Transform ---
    // -----------------

    Transform::Transform(GameObject* owner) : m_Owner(owner) { }

    glm::vec2 Transform::GetLocalPos() const
    {
        return m_LocalPosition;
    }

    void Transform::SetLocalPos(const float x, const float y)
    {
        m_LocalPosition.x = x;
        m_LocalPosition.y = y;

        MarkWorldPosDirty();
    }

    void Transform::SetLocalPos(const glm::vec2& position)
    {
        m_LocalPosition = position;

        MarkWorldPosDirty();
    }

    glm::vec2 Transform::GetWorldPos() const
    {
        GameObject* parent = m_Owner->GetParent();

        if (m_WorldPosDirty)
        {
            if (parent)
            {
                m_WorldPosition = m_LocalPosition +
                    m_Owner->GetParent()->GetTransform().GetWorldPos();
            }
            else
            {
                m_WorldPosition = m_LocalPosition;
            }

            m_WorldPosDirty = false;
        }

        return m_WorldPosition;
    }

    void Transform::MarkWorldPosDirty()
    {
        m_WorldPosDirty = true;

        for (auto& child : m_Owner->GetChildren())
        {
            child->GetTransform().MarkWorldPosDirty();
        }
    }

    void Transform::Parse(tinyxml2::XMLElement* element)
    {
        glm::vec2 pos { };

        if (auto* positionElem { element->FirstChildElement("position") })
        {
            std::istringstream iss(positionElem->GetText());

            if (!(iss >> pos.x >> pos.y))
            {
                logError("Failed to parse transform position, value: '{}'", positionElem->GetText());
            }
        }

        SetLocalPos(pos);
    }



    // -----------------
    // --- Component ---
    // -----------------

    GameObject* Component::GetOwner() const
    {
        return m_Owner;
    }

    Component::ComponentFactory& Component::GetComponentFactory()
    {
        return GetFactory();
    }

    void Component::Parse(Component* component, tinyxml2::XMLElement* componentElement)
    {
        auto& registry = GetRegistry()[typeid(*component)];

        for (const auto& [typeId, parserRegistry] : registry)
        {
            for (const auto& [name, parser] : parserRegistry)
            {
                tinyxml2::XMLElement* parameterElement = componentElement->FirstChildElement(name.c_str());

                if (parameterElement == nullptr)
                {
                    logWarning("Parameter with name '{}' not found", name);

                    continue;
                }

                parser(component, std::string(parameterElement->GetText()));
            }
        }
    }



    // ------------------
    // --- GameObject ---
    // ------------------

    void GameObject::OnInit(EngineCtx& ctx)
    {
        for (auto& [type, component] : m_ComponentMap)
        {
            component->OnInit(ctx);
        }
    }

    void GameObject::Update(EngineCtx& ctx)
    {
        for (auto& [type, component] : m_ComponentMap)
        {
            component->Update(ctx);
        }
    }

    void GameObject::FixedUpdate(EngineCtx& ctx)
    {
        for (auto& [type, component] : m_ComponentMap)
        {
            component->FixedUpdate(ctx);
        }
    }

    void GameObject::OnDestroy(EngineCtx& ctx)
    {
        for (auto& [type, component] : m_ComponentMap)
        {
            component->OnDestroy(ctx);
        }
    }

    Transform& GameObject::GetTransform()
    {
        return m_Transform;
    }

    GameObject* GameObject::GetParent() const
    {
        return m_Parent;
    }

    void GameObject::SetParent(GameObject* parent, bool keepWorldPos)
    {
        if (IsChild(parent))
        {
            assert(false && "Attempted to set a child object as a parent");

            return;
        }

        if (parent == this || m_Parent == parent)
            return;

        if (parent == nullptr)
        {
            m_Transform.SetLocalPos(m_Transform.GetWorldPos());
        }
        else
        {
            if (keepWorldPos)
            {
                m_Transform.SetLocalPos(m_Transform.GetWorldPos() -
                    parent->GetTransform().GetWorldPos());
            }
            else
            {
                m_Transform.MarkWorldPosDirty();
            }
        }

        if (m_Parent)
        {
            auto it { std::find(m_Parent->m_Children.begin(),
                m_Parent->m_Children.end(), this) };

            if (it != m_Parent->m_Children.end())
                m_Parent->m_Children.erase(it);
            else
                assert(false && "Object not found in its parent's children");
        }

        m_Parent = parent;

        if (m_Parent)
        {
            m_Parent->m_Children.push_back(this);
        }
    }

    unsigned int GameObject::GetChildCount() const
    {
        return static_cast<unsigned int>(m_Children.size());
    }

    GameObject* GameObject::GetChildById(unsigned int id) const
    {
        if (id >= m_Children.size())
        {
            assert(false && "Child index out of range");

            return nullptr;
        }

        return m_Children[id];
    }

    const std::vector<GameObject*>& GameObject::GetChildren() const
    {
        return m_Children;
    }

    bool GameObject::IsChild(GameObject* obj) const
    {
        if (!obj) return false;

        return std::any_of(
            m_Children.begin(),
            m_Children.end(),
            [obj](GameObject* child)
        {
            return child == obj || child->IsChild(obj);
        }
        );
    }

    void GameObject::Parse(GameObject* object, tinyxml2::XMLElement* element,
        std::vector<tinyxml2::XMLElement*>& childElements)
    {
        if (auto* childrenElement { element->FirstChildElement("children") })
        {
            for (auto childElement = childrenElement->FirstChildElement("object");
                childElement != nullptr;
                childElement = childElement->NextSiblingElement("object"))
            {
                childElements.push_back(childElement);
            }
        }

        if (auto* transformElement { element->FirstChildElement("transform") })
        {
            object->GetTransform().Parse(transformElement);
        }
        else
        {
            assert(false && "Parsed game object does not have transform!");
        }

        if (auto* componentsElement { element->FirstChildElement("components") })
        {
            auto& componentFactory = Component::GetComponentFactory();

            for (auto componentElement = componentsElement->FirstChildElement();
                componentElement != nullptr;
                componentElement = componentElement->NextSiblingElement())
            {
                auto it { componentFactory.find(componentElement->Name()) };

                if (it == componentFactory.end())
                {
                    logError("Unknown component type '{}'", componentElement->Name());

                    continue;
                }

                // Calls AddComponentNoInit on an object for the respective component type
                auto& componentMakerFunc { it->second };

                Component* comp { componentMakerFunc(object) };

                Component::Parse(comp, componentElement);
            }
        }
    }

    void GameObject::RemoveChildrenMarkedForDeletion()
    {
        for (auto it { m_Children.begin() }; it != m_Children.end();)
        {
            if ((*it)->m_MarkedForDeletion)
                it = m_Children.erase(it);
            else
                it++;
        }
    }

    void GameObject::PropagateMarkedForDeletion()
    {
        if (!m_MarkedForDeletion)
            return;

        for (auto& child : m_Children)
        {
            child->m_MarkedForDeletion = true;
        }
    }
}
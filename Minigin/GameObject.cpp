#include "GameObject.h"

#include <algorithm>



dae::Transform::Transform(dae::GameObject* owner) : m_Owner(owner) { }

glm::vec3 dae::Transform::GetLocalPos() const
{
    return m_LocalPosition;
}

void dae::Transform::SetLocalPos(const float x, const float y, const float z)
{
    m_LocalPosition.x = x;
    m_LocalPosition.y = y;
    m_LocalPosition.z = z;

    MarkWorldPosDirty();
}

void dae::Transform::SetLocalPos(const glm::vec3& position)
{
    m_LocalPosition = position;

    MarkWorldPosDirty();
}

glm::vec3 dae::Transform::GetWorldPos() const
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

void dae::Transform::MarkWorldPosDirty()
{
    m_WorldPosDirty = true;

    for (auto& child : m_Owner->GetChildren())
    {
        child->GetTransform().MarkWorldPosDirty();
    }
}



dae::GameObject* dae::Component::GetOwner() const
{
    return m_Owner;
}



void dae::GameObject::Initialize()
{
    for (auto& [type, component] : m_ComponentMap)
    {
        component->Initialize();
    }
}

void dae::GameObject::Update(float deltaTime)
{
    for (auto& [type, component] : m_ComponentMap)
    {
        component->Update(deltaTime);
    }
}

void dae::GameObject::FixedUpdate(float deltaTime)
{
    for (auto& [type, component] : m_ComponentMap)
    {
        component->FixedUpdate(deltaTime);
    }
}

void dae::GameObject::Render() const
{
    for (auto& [type, component] : m_ComponentMap)
    {
        component->Render();
    }
}

dae::Transform& dae::GameObject::GetTransform()
{
    return m_Transform;
}

dae::GameObject* dae::GameObject::GetParent() const
{
    return m_Parent;
}

void dae::GameObject::SetParent(GameObject* parent, bool keepWorldPos)
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

unsigned int dae::GameObject::GetChildCount() const
{
    return static_cast<unsigned int>(m_Children.size());
}

dae::GameObject* dae::GameObject::GetChildById(unsigned int id) const
{
    if (id >= m_Children.size())
    {
        assert(false && "Child index out of range");

        return nullptr;
    }

    return m_Children[id];
}

const std::vector<dae::GameObject*>& dae::GameObject::GetChildren() const
{
    return m_Children;
}

bool dae::GameObject::IsChild(GameObject* obj) const
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

bool dae::GameObject::IsMarkedForDeletion() const
{
    return m_MarkedForDeletion;
}

void dae::GameObject::MarkForDeletion()
{
    m_MarkedForDeletion = true;

    for (auto& child : m_Children)
    {
        child->MarkForDeletion();
    }
}

void dae::GameObject::RemoveChildrenMarkedForDeletion()
{
    for (auto it { m_Children.begin() }; it != m_Children.end();)
    {
        if ((*it)->IsMarkedForDeletion())
            it = m_Children.erase(it);
        else
            it++;
    }
}
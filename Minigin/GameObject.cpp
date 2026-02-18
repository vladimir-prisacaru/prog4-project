#include "GameObject.h"
#include "ResourceManager.h"
#include "Renderer.h"

#include <string>
#include <iostream>



dae::GameObject::~GameObject() = default;

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

glm::vec2 dae::GameObject::GetPosition()
{
    return m_Transform.GetPosition();
}

void dae::GameObject::SetPosition(float x, float y)
{
    m_Transform.SetPosition(x, y, 0.0f);
}
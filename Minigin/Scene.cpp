#include <algorithm>
#include "Scene.h"

using namespace dae;

void Scene::Add(std::unique_ptr<GameObject> object)
{
    assert(object != nullptr && "Cannot add a null GameObject to the scene.");

    m_Objects.emplace_back(std::move(object));
}

void Scene::Remove(const GameObject& object)
{
    for (const auto& obj : m_Objects)
    {
        if (obj.get() == &object)
            obj->MarkForDeletion();
    }
}

void Scene::RemoveAll()
{
    for (const auto& obj : m_Objects)
    {
        obj->MarkForDeletion();
    }
}

void Scene::Update(float deltaTime)
{
    for (auto& object : m_Objects)
    {
        object->Update(deltaTime);
    }
}

void Scene::FixedUpdate(float deltaTime)
{
    for (auto& object : m_Objects)
    {
        object->FixedUpdate(deltaTime);
    }
}

void Scene::Render() const
{
    for (const auto& object : m_Objects)
    {
        object->Render();
    }
}

void Scene::CleanupMarked()
{
    for (auto& obj : m_Objects)
    {
        obj->RemoveChildrenMarkedForDeletion();
    }

    for (auto it { m_Objects.begin() }; it != m_Objects.end();)
    {
        if ((*it)->IsMarkedForDeletion())
            it = m_Objects.erase(it);
        else
            it++;
    }
}
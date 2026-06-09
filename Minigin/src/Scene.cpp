#include <stack>

#include "Scene.h"



namespace dae
{
    Scene::~Scene() { };

    GameObject* Scene::Instantiate()
    {
        m_Objects.emplace_back(new GameObject());
        m_Ctx.deltaTime = 0.0f;
        m_Objects.back()->m_Ctx = m_Ctx;
        return m_Objects.back().get();
    }

    void Scene::Destroy(GameObject* object)
    {
        auto it = std::find_if(
            m_Objects.begin(),
            m_Objects.end(),
            [object](const std::unique_ptr<GameObject>& ptr)
            {
                return ptr.get() == object;
            }
        );

        if (it == m_Objects.end())
        {
            assert(false && "Tried to destroy an invalid GameObject!");

            return;
        }

        (*it)->m_MarkedForDeletion = true;
    }

    void Scene::Parse(Scene* scene, tinyxml2::XMLElement* sceneElement)
    {
        std::stack<std::pair<GameObject*, tinyxml2::XMLElement*>> stack { };

        // put root objects onto the stack
        for (auto* objElem = sceneElement->FirstChildElement("object");
            objElem != nullptr;
            objElem = objElem->NextSiblingElement("object"))
        {
            auto* obj = scene->Instantiate();

            stack.push({ obj, objElem });
        }

        // parse all objects, pushing children back onto the stack
        while (!stack.empty())
        {
            auto [obj, objElem] { stack.top() };
            stack.pop();

            std::vector<tinyxml2::XMLElement*> childElements;

            GameObject::Parse(obj, objElem, childElements);

            for (auto& childElem : childElements)
            {
                auto* childObj { scene->Instantiate() };

                childObj->SetParent(obj);

                stack.push({ childObj, childElem });
            }
        }
    }

    void Scene::Init()
    {
        for (auto& obj : m_Objects)
        {
            m_Ctx.deltaTime = 0.0f;
            obj->OnInit(m_Ctx);
        }
    }

    void Scene::Update(float deltaTime)
    {
        CleanupDestroyedObjects();

        m_Ctx.deltaTime = deltaTime;

        for (auto& object : m_Objects)
        {
            object->Update(m_Ctx);
        }
    }

    void Scene::FixedUpdate(float deltaTime)
    {
        m_Ctx.deltaTime = deltaTime;

        for (auto& object : m_Objects)
        {
            object->FixedUpdate(m_Ctx);
        }
    }

    void Scene::OnUnload()
    {
        for (auto& obj : m_Objects)
            obj->m_MarkedForDeletion = true;

        CleanupDestroyedObjects();
    }

    void Scene::CleanupDestroyedObjects()
    {
        for (auto& obj : m_Objects)
        {
            obj->PropagateMarkedForDeletion();
        }

        for (auto& obj : m_Objects)
        {
            if (obj->m_MarkedForDeletion)
                obj->OnDestroy(m_Ctx);
        }

        for (auto& obj : m_Objects)
        {
            obj->RemoveChildrenMarkedForDeletion();
        }

        for (auto it { m_Objects.begin() }; it != m_Objects.end();)
        {
            if ((*it)->m_MarkedForDeletion)
            {
                it = m_Objects.erase(it);
            }
            else
                it++;
        }
    }
}
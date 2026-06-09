#pragma once

#include <glm/glm.hpp>

namespace dae
{
    class GameObject;
    class ICollider;

    struct Ray
    {
        glm::vec2 pos { };
        glm::vec2 dir { };
    };

    struct RaycastHit
    {
        bool hit { };
        float t { };
        glm::vec2 pos { };
        glm::vec2 normal { };
        ICollider* collider { };
        GameObject* obj { };
    };

    class ICollider
    {
        public:

        virtual ~ICollider() = default;

        virtual RaycastHit Raycast(Ray ray, float maxDist) = 0;
        virtual bool CheckOverlap(ICollider* other, bool& supported) = 0;
    };
}
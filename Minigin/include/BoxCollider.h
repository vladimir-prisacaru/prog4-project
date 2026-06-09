#pragma once

#include "GameObject.h"
#include "ICollider.h"

namespace dae
{
    struct AABB
    {
        glm::vec2 min { };
        glm::vec2 max { };
    };

    class BoxCollider : public Component, public Registrar<BoxCollider>, public ICollider
    {
        public:

        static void Register();

        explicit BoxCollider(GameObject* owner) : Component(owner) { };
        virtual ~BoxCollider() = default;

        void OnInit(EngineCtx& ctx);
        void OnDestroy(EngineCtx& ctx);

        glm::vec2 GetCenter() const;
        void SetCenter(glm::vec2 center);
        glm::vec2 GetExtents() const;
        void SetExtents(glm::vec2 extents);

        RaycastHit Raycast(Ray ray, float maxDist) override;
        bool CheckOverlap(ICollider* other, bool& supported) override;

        /* Returns the world-space axis-aligned bounding box */
        AABB GetAABB() const;

        private:

        glm::vec2 m_Extents { };
        glm::vec2 m_Center { };
    };
}
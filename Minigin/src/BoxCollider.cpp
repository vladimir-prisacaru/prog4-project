#include "BoxCollider.h"
#include "Physics.h"

namespace dae
{
    void BoxCollider::Register()
    {
        RegisterComponent<BoxCollider>("box_collider");

        RegisterParameter("center", &BoxCollider::m_Center);
        RegisterParameter("extents", &BoxCollider::m_Extents);
    }

    void BoxCollider::OnInit(EngineCtx& ctx)
    {
        ctx.physics->AddCollider(this);
    }

    void BoxCollider::OnDestroy(EngineCtx& ctx)
    {
        ctx.physics->RemoveCollider(this);
    }

    glm::vec2 BoxCollider::GetCenter() const
    {
        return m_Center;
    }

    void BoxCollider::SetCenter(glm::vec2 center)
    {
        m_Center = center;
    }

    glm::vec2 BoxCollider::GetExtents() const
    {
        return m_Extents;
    }

    void BoxCollider::SetExtents(glm::vec2 extents)
    {
        m_Extents = extents;
    }

    RaycastHit BoxCollider::Raycast(Ray ray, float maxDist)
    {
        const AABB box { GetAABB() };

        float tMin { 0.f };
        float tMax { maxDist };

        int hitAxis { -1 }; // 0 = X slab, 1 = Y slab
        float hitSign { 0.f }; // sign of the normal on the hit axis

        for (int axis { 0 }; axis < 2; ++axis)
        {
            const float origin { ray.pos[axis] };
            const float direction { ray.dir[axis] };
            const float slabMin { box.min[axis] };
            const float slabMax { box.max[axis] };

            if (std::abs(direction) < std::numeric_limits<float>::epsilon())
            {
                // Ray is parallel to this slab, check if origin is inside it
                if (origin < slabMin || origin > slabMax)
                    return RaycastHit { };  // misses entirely

                continue;
            }

            const float invDir { 1.f / direction };
            float t0 { (slabMin - origin) * invDir };
            float t1 { (slabMax - origin) * invDir };

            // t0 should be the entry, t1 the exit
            float entrySign { -1.f };
            if (t0 > t1)
            {
                std::swap(t0, t1);
                entrySign = 1.f;
            }

            if (t0 > tMin)
            {
                tMin = t0;
                hitAxis = axis;
                hitSign = entrySign;
            }

            tMax = std::min(tMax, t1);

            if (tMin > tMax)
                return RaycastHit { };  // slabs don't overlap, miss
        }

        // tMin < 0 means the ray origin is inside the box.
        // Whether that counts as a hit is context-dependent; here we treat it
        // as a miss (the ray is not pointing at a surface it can hit from outside)
        if (tMin < 0.f)
            return RaycastHit { };

        RaycastHit result { };
        result.hit = true;
        result.t = tMin;
        result.pos = ray.pos + ray.dir * tMin;
        result.collider = this;
        result.obj = GetOwner();

        // Build the outward surface normal from the entry axis and sign
        result.normal = glm::vec2 { 0.f, 0.f };
        result.normal[hitAxis] = hitSign;

        return result;
    }

    bool BoxCollider::CheckOverlap(ICollider* other, bool& supported)
    {
        if (BoxCollider* otherBox { dynamic_cast<BoxCollider*>(other) })
        {
            supported = true;

            const AABB a { GetAABB() };
            const AABB b { otherBox->GetAABB() };

            return
                a.max.x > b.min.x &&
                a.min.x < b.max.x &&
                a.max.y > b.min.y &&
                a.min.y < b.max.y;
        }

        supported = false;
        return false;
    }

    AABB BoxCollider::GetAABB() const
    {
        const glm::vec2 worldCenter { GetTransform().GetWorldPos() + m_Center };

        return AABB
        {
            worldCenter - m_Extents,
            worldCenter + m_Extents,
        };
    }
}
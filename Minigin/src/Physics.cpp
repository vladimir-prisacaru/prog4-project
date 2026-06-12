#include <algorithm>

#include "Physics.h"
#include "ICollisionReceiver.h"
#include "GameObject.h"

namespace dae
{
    Physics::Physics() = default;

    Physics::~Physics() = default;

    void Physics::AddCollider(ICollider* collider)
    {
        m_Colliders.push_back(collider);
    }

    void Physics::RemoveCollider(ICollider* collider)
    {
        auto it { std::find(m_Colliders.begin(), m_Colliders.end(), collider) };

        if (it == m_Colliders.end())
        {
            assert(false && "Tried to remove an invalid ICollider");

            return;
        }

        m_Colliders.erase(it);

        // Fire OnOverlapEnd for every active pair that involves this collider,
        // then remove those pairs so Update() never touches the now-dangling pointer.
        for (auto pairIt { m_ActiveOverlaps.begin() }; pairIt != m_ActiveOverlaps.end();)
        {
            if (pairIt->first != collider && pairIt->second != collider)
            {
                ++pairIt;
                continue;
            }

            ICollider* other { pairIt->first == collider ? pairIt->second : pairIt->first };

            // Notify the surviving side that the overlap ended
            if (auto* componentOther { dynamic_cast<Component*>(other) })
            {
                if (auto* receiver { componentOther->GetComponentOfType<ICollisionReceiver>() })
                    receiver->OnOverlapEnd(collider);
            }

            pairIt = m_ActiveOverlaps.erase(pairIt);
        }
    }



    // --- Raycasting ---

    RaycastHit Physics::Raycast(Ray ray, float maxDist)
    {
        // TODO: implement with parallel execution

        RaycastHit closest { };

        for (auto* collider : m_Colliders)
        {
            if (!collider->IsEnabled())
                continue;

            RaycastHit hit { collider->Raycast(ray, maxDist) };

            if (!hit.hit)
                continue;

            if (hit.t > closest.t)
                continue;

            closest = std::move(hit);
        }

        return closest;
    }

    std::vector<RaycastHit> Physics::RaycastAll(Ray ray, float maxDist, bool sort)
    {
        // TODO: implement with parallel execution
        std::vector<RaycastHit> hits { };

        for (auto* collider : m_Colliders)
        {
            if (!collider->IsEnabled())
                continue;

            RaycastHit hit { collider->Raycast(ray, maxDist) };

            if (!hit.hit)
                continue;

            hits.push_back(std::move(hit));
        }

        if (!sort)
            return hits;

        std::sort(hits.begin(), hits.end(),
            [] (const RaycastHit& lhs, const RaycastHit& rhs) -> bool
            {
                return lhs.t < rhs.t;
            });

        return hits;
    }



    // --- Collision events ---

    void Physics::Update()
    {
        std::set<ColliderPair> currentOverlaps { };

        // Iterate over every collider pair :(
        for (std::size_t i { 0 }; i < m_Colliders.size(); ++i)
        {
            for (std::size_t j { i + 1 }; j < m_Colliders.size(); ++j)
            {
                ICollider* a { m_Colliders[i] };
                ICollider* b { m_Colliders[j] };

                if (!a->IsEnabled() || !b->IsEnabled())
                    continue;

                {
                    bool supported { };

                    if (!a->CheckOverlap(b, supported))
                    {
                        if (supported)
                            continue;

                        if (!b->CheckOverlap(a, supported))
                            continue;
                    }
                }

                currentOverlaps.insert(MakePair(a, b));

                // Skip pairs that were overlapping last frame
                if (m_ActiveOverlaps.contains(MakePair(a, b)))
                    continue;

                // Notify both sides if they have a component that implements ICollisionReceiver
                if (auto* componentA { dynamic_cast<Component*>(a) })
                {
                    if (auto* receiverA { componentA->GetComponentOfType<ICollisionReceiver>() })
                        receiverA->OnOverlap(b);
                }

                if (auto* componentB { dynamic_cast<Component*>(b) })
                {
                    if (auto* receiverB { componentB->GetComponentOfType<ICollisionReceiver>() })
                        receiverB->OnOverlap(a);
                }
            }
        }

        // Call OnOverlapEnd for pairs that were overlapping last frame but aren't now
        for (const ColliderPair& pair : m_ActiveOverlaps)
        {
            if (currentOverlaps.contains(pair))
                continue;

            if (auto* componentA { dynamic_cast<Component*>(pair.first) })
            {
                if (auto* receiverA { componentA->GetComponentOfType<ICollisionReceiver>() })
                    receiverA->OnOverlapEnd(pair.second);
            }

            if (auto* componentB { dynamic_cast<Component*>(pair.second) })
            {
                if (auto* receiverB { componentB->GetComponentOfType<ICollisionReceiver>() })
                    receiverB->OnOverlapEnd(pair.first);
            }
        }

        m_ActiveOverlaps = std::move(currentOverlaps);
    }
}
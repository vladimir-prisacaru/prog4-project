#pragma once

#include <vector>
#include <set>
#include <type_traits>

#include "ICollider.h"

namespace dae
{
    class Minigin;

    class Physics final
    {
        public:

        Physics();
        ~Physics();

        void AddCollider(ICollider* collider);
        void RemoveCollider(ICollider* collider);

        /* Raycasts against all colliders and returns the closest hit */
        RaycastHit Raycast(Ray ray, float maxDist);
        /* Raycasts against all colliders and returns all hits */
        std::vector<RaycastHit> RaycastAll(Ray ray, float maxDist, bool sort = false);

        /* Raycasts against all colliders of a certain type and returns the closest hit */
        template<typename T>
        RaycastHit Raycast(Ray ray, float maxDist, T** hitObj = nullptr)
            requires std::is_base_of_v<ICollider, T>
        {
            RaycastHit closest { };
            T* obj { };

            for (auto* collider : m_Colliders)
            {
                obj = dynamic_cast<T*>(collider);

                if (obj == nullptr)
                    continue;

                RaycastHit hit { collider->Raycast(ray, maxDist) };

                if (!hit.hit)
                    continue;

                if (hit.t > closest.t)
                    continue;

                closest = std::move(hit);
            }

            if (hitObj != nullptr && closest.hit)
                *hitObj = obj;

            return closest;
        }

        /* Raycasts against all colliders of a certain type and returns all hits */
        template<typename T>
        std::vector<RaycastHit> RaycastAll(Ray ray, float maxDist, bool sort = false)
            requires std::is_base_of_v<ICollider, T>
        {
            std::vector<RaycastHit> hits { };

            for (auto* collider : m_Colliders)
            {
                if (dynamic_cast<T*>(collider) == nullptr)
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

        private:

        friend class Minigin;

        /* Must be called once per frame to detect and dispatch overlap events */
        void Update();

        std::vector<ICollider*> m_Colliders;

        // Canonical pair: always store (lower ptr, higher ptr) so (A,B) == (B,A)
        using ColliderPair = std::pair<ICollider*, ICollider*>;

        static ColliderPair MakePair(ICollider* a, ICollider* b)
        {
            return (a < b) ? ColliderPair { a, b } : ColliderPair { b, a };
        }

        std::set<ColliderPair> m_ActiveOverlaps; // pairs currently overlapping
    };
}
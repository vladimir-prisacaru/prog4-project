#pragma once

#include "GameObject.h"
#include "ICollider.h"

namespace dae
{
    class GridComponent;

    class GridCollider : public Component, public Registrar<GridCollider>, public ICollider
    {
        public:

        static void Register();

        explicit GridCollider(GameObject* owner) : Component(owner) { };
        ~GridCollider() override = default;

        void OnInit(EngineCtx& ctx) override;
        void OnDestroy(EngineCtx& ctx) override;

        bool IsEnabled() override { return m_IsEnabled; }
        void SetEnabled(bool isEnabled) override { m_IsEnabled = isEnabled; }

        RaycastHit Raycast(Ray ray, float maxDist) override;

        // No overlap support
        bool CheckOverlap(ICollider*, bool& supported) override { supported = false; return false; }

        private:

        bool m_IsEnabled { true };

        // Tile ids that block rays
        std::vector<int> m_SolidTileIds { };

        GridComponent* m_Grid { };
    };
}
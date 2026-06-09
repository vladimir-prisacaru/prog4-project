#pragma once

#include "GameObject.h"
#include "IRenderable.h"
#include "GridComponent.h"
#include "Texture2D.h"

namespace dae
{
    class GridRenderer : public Component, public Registrar<GridRenderer>, public IRenderable
    {
        public:

        static void Register();

        explicit GridRenderer(GameObject* owner) : Component(owner) { };
        virtual ~GridRenderer() = default;

        void OnInit(EngineCtx& ctx) override;
        void OnDestroy(EngineCtx& ctx) override;

        int GetDrawOrder() const override;
        void Render(const Renderer* renderer) const override;

        private:

        // Params
        std::string m_SpritesheetPath { };
        int m_TileSrcSize { };
        int m_DrawOrder { };

        // Other
        GridComponent* m_Grid { };
        std::shared_ptr<Texture2D> m_Spritesheet { };
    };
}
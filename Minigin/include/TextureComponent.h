#pragma once

#include "GameObject.h"
#include "Texture2D.h"
#include "IRenderable.h"

namespace dae
{
    class TextureComponent : public Component, public Registrar<TextureComponent>, public IRenderable
    {
        public:

        static void Register();

        TextureComponent(GameObject* owner) : Component(owner) { }
        virtual ~TextureComponent() = default;

        void OnInit(EngineCtx& ctx) override;
        void OnDestroy(EngineCtx& ctx) override;

        int GetDrawOrder() const override;
        void Render(const Renderer* renderer) const override;

        void SetTexture(std::shared_ptr<Texture2D> texture);
        void SetTexture(const std::string& file);

        private:

        std::string m_TexturePath { };
        int m_DrawOrder { };
        float m_Scale { 1.0f };

        std::shared_ptr<Texture2D> m_Texture { };

        ResourceManager* m_ResourceManager { };
    };
}
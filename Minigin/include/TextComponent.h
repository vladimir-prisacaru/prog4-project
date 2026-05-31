#pragma once

#include <SDL3/SDL_pixels.h>

#include "GameObject.h"
#include "Font.h"
#include "TextureComponent.h"

namespace dae
{
    class TextComponent : public Component, public Registrar<TextComponent>
    {
        public:

        static void Register();

        TextComponent(GameObject* owner) : Component(owner) { };
        virtual ~TextComponent() = default;

        void OnInit(EngineCtx& ctx) override;
        void Update(EngineCtx& ctx) override;

        void SetText(const std::string& text);
        void SetColor(const SDL_Color& color);

        void SetFont(std::shared_ptr<Font> font);
        void SetFont(const std::string& file, uint8_t size);

        private:

        std::string m_FontPath { };
        int m_FontSize { };
        std::string m_TextStr { "none" };
        SDL_Color m_TextColor { 255, 255, 255, 255 };

        bool m_NeedsUpdate { };

        std::shared_ptr<Font> m_Font { };
        TextureComponent* m_TextureComponent { };

        ResourceManager* m_ResourceManager { };
    };
}
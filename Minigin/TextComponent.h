#pragma once

#include <SDL3/SDL_pixels.h>

#include "GameObject.h"
#include "Font.h"
#include "Texture2D.h"

namespace dae
{
    class TextComponent : public Component
    {
        public:

        TextComponent(GameObject* parent) : Component(parent) { }
        virtual ~TextComponent() = default;

        void Update(float deltaTime) override;
        void Render() const override;

        void SetText(const std::string& text);
        void SetPosition(float x, float y);
        void SetColor(const SDL_Color& color);

        void SetFont(std::shared_ptr<Font> font);
        void SetFont(const std::string& file, uint8_t size);

        private:

        bool m_NeedsUpdate { };
        std::string m_Text { };
        SDL_Color m_Color { 255, 255, 255, 255 };

        std::shared_ptr<Font> m_Font { };
        std::shared_ptr<Texture2D> m_TextTexture { };
    };
}
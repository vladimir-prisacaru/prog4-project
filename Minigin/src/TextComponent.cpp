#include <stdexcept>
#include <SDL3_ttf/SDL_ttf.h>

#include "TextComponent.h"
#include "Renderer.h"
#include "ResourceManager.h"

void dae::TextComponent::Register()
{
    RegisterComponent<TextComponent>("text_component");

    RegisterParameter("font_path", &TextComponent::m_FontPath);
    RegisterParameter("font_size", &TextComponent::m_FontSize);
    RegisterParameter("text_str", &TextComponent::m_TextStr);
    RegisterParameter("text_color", &TextComponent::m_TextColor);
}

void dae::TextComponent::SetText(const std::string& text)
{
    m_TextStr = text;
    m_NeedsUpdate = true;
}

void dae::TextComponent::SetColor(const SDL_Color& color)
{
    m_TextColor = color;
    m_NeedsUpdate = true;
}

void dae::TextComponent::SetFont(std::shared_ptr<Font> font)
{
    m_Font = font;
    m_NeedsUpdate = true;
}

void dae::TextComponent::SetFont(const std::string& file, uint8_t size)
{
    SetFont(m_ResourceManager->LoadFont(file, size));
}

void dae::TextComponent::OnInit(EngineCtx& ctx)
{
    m_ResourceManager = ctx.resourceManager;

    if (!m_FontPath.empty())
        SetFont(m_FontPath, static_cast<uint8_t>(m_FontSize));

    SetText(m_TextStr);
    SetColor(m_TextColor);
}

void dae::TextComponent::Update(EngineCtx& ctx)
{
    if (m_TextureComponent == nullptr)
    {
        auto texture { GetOwner()->GetComponent<TextureComponent>() };

        if (texture == nullptr)
            return;

        m_TextureComponent = texture;
    }

    if (!m_NeedsUpdate)
        return;

    if (m_Font->GetFont() == nullptr)
        return;

    const auto surf { TTF_RenderText_Blended(m_Font->GetFont(),
        m_TextStr.c_str(), m_TextStr.length(), m_TextColor) };

    if (surf == nullptr)
        throw std::runtime_error(std::string("Render text failed: ") +
            SDL_GetError());

    auto texture { SDL_CreateTextureFromSurface(
        ctx.renderer->GetSDLRenderer(), surf) };

    if (texture == nullptr)
        throw std::runtime_error(
            std::string("Create text texture from surface failed: ") +
            SDL_GetError());

    SDL_DestroySurface(surf);

    m_TextureComponent->SetTexture(std::make_shared<Texture2D>(texture));

    m_NeedsUpdate = false;
}
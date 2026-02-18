#include <stdexcept>
#include <SDL3_ttf/SDL_ttf.h>

#include "TextComponent.h"
#include "Renderer.h"
#include "ResourceManager.h"



void dae::TextComponent::SetText(const std::string& text)
{
    m_Text = text;
    m_NeedsUpdate = true;
}

void dae::TextComponent::SetPosition(const float x, const float y)
{
    m_Parent->SetPosition(x, y);
}

void dae::TextComponent::SetColor(const SDL_Color& color)
{
    m_Color = color;
    m_NeedsUpdate = true;
}

void dae::TextComponent::SetFont(std::shared_ptr<Font> font)
{
    m_Font = font;
    m_NeedsUpdate = true;
}

void dae::TextComponent::SetFont(const std::string& file, uint8_t size)
{
    SetFont(dae::ResourceManager::GetInstance().LoadFont(file, size));
}

void dae::TextComponent::Update(float deltaTime)
{
    std::ignore = deltaTime;

    if (!m_NeedsUpdate)
        return;

    if (m_Font->GetFont() == nullptr)
        return;

    const auto surf { TTF_RenderText_Blended(m_Font->GetFont(),
        m_Text.c_str(), m_Text.length(), m_Color) };

    if (surf == nullptr)
        throw std::runtime_error(std::string("Render text failed: ") +
            SDL_GetError());

    auto texture { SDL_CreateTextureFromSurface(
        Renderer::GetInstance().GetSDLRenderer(), surf) };

    if (texture == nullptr)
        throw std::runtime_error(
            std::string("Create text texture from surface failed: ") +
            SDL_GetError());

    SDL_DestroySurface(surf);

    m_TextTexture = std::make_shared<Texture2D>(texture);
    m_NeedsUpdate = false;
}

void dae::TextComponent::Render() const
{
    if (m_TextTexture == nullptr)
        return;

    const auto& pos { m_Parent->GetPosition() };

    Renderer::GetInstance().RenderTexture(*m_TextTexture, pos.x, pos.y);
}
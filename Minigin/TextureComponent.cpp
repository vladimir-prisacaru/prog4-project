#include "TextureComponent.h"
#include "Renderer.h"
#include "ResourceManager.h"



void dae::TextureComponent::SetTexture(std::shared_ptr<Texture2D> texture)
{
    m_Texture = texture;
}

void dae::TextureComponent::SetTexture(const std::string& file)
{
    SetTexture(dae::ResourceManager::GetInstance().LoadTexture(file));
}

void dae::TextureComponent::Render() const
{
    if (m_Texture == nullptr)
        return;

    const auto& pos { GetOwner()->GetTransform().GetWorldPos() };
    Renderer::GetInstance().RenderTexture(*m_Texture, pos.x, pos.y);
}
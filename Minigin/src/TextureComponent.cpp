#include "TextureComponent.h"
#include "Renderer.h"
#include "ResourceManager.h"

void dae::TextureComponent::Register()
{
    RegisterComponent<TextureComponent>("texture_component");

    RegisterParameter("texture_path", &TextureComponent::m_TexturePath);
    RegisterParameter("draw_order", &TextureComponent::m_DrawOrder);
}

void dae::TextureComponent::SetTexture(std::shared_ptr<Texture2D> texture)
{
    m_TexturePath = "none";
    m_Texture = texture;
}

void dae::TextureComponent::SetTexture(const std::string& file)
{
    m_TexturePath = file;
    SetTexture(m_ResourceManager->LoadTexture(file));
}

void dae::TextureComponent::OnInit(EngineCtx& ctx)
{
    ctx.renderer->AddRenderable(this);

    m_ResourceManager = ctx.resourceManager;

    if (!m_TexturePath.empty())
        SetTexture(m_TexturePath);
}

void dae::TextureComponent::OnDestroy(EngineCtx& ctx)
{
    ctx.renderer->RemoveRenderable(this);
}

int dae::TextureComponent::GetDrawOrder() const
{
    return m_DrawOrder;
}

void dae::TextureComponent::Render(const Renderer* renderer) const
{
    if (m_Texture == nullptr)
        return;

    const auto& pos { GetOwner()->GetTransform().GetWorldPos() };
    renderer->RenderTexture(*m_Texture, pos.x, pos.y);
}
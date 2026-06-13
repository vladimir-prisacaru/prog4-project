#include <nlohmann/json.hpp>

#include "SpriteComponent.h"
#include "Renderer.h"
#include "ResourceManager.h"

using json = nlohmann::json;



namespace dae
{

    void SpriteComponent::Register()
    {
        RegisterComponent<SpriteComponent>("sprite_component");

        RegisterParameter("spritesheet_path", &SpriteComponent::m_SpritesheetPath);
        RegisterParameter("animations_path", &SpriteComponent::m_AnimationsPath);
        RegisterParameter("draw_order", &SpriteComponent::m_DrawOrder);
        RegisterParameter("scale", &SpriteComponent::m_Scale);
    }

    void SpriteComponent::OnInit(EngineCtx& ctx)
    {
        ctx.renderer->AddRenderable(this);

        // Import spritesheet texture
        m_Spritesheet = ctx.resourceManager->LoadTexture(m_SpritesheetPath, true);

        // Parse animations
        auto fullPath { ctx.dataPath / m_AnimationsPath };
        std::ifstream file(fullPath);

        if (!file.is_open())
        {
            logError("Animation parse error: could not open '{}'", fullPath.string());

            return;
        }

        json root;

        try
        {
            root = json::parse(file);
        }
        catch (const json::parse_error& e)
        {
            logError("Animation parse error: JSON parse error in '{}': {}", fullPath.string(), e.what());

            return;
        }

        try
        {
            for (const auto& [animName, animData] : root.items())
            {
                Animation anim { };
                anim.isLooping = animData.at("isLooping").get<bool>();

                for (const auto& frameData : animData.at("frames"))
                {
                    Frame frame { };

                    frame.top = frameData.at("top").get<int>();
                    frame.left = frameData.at("left").get<int>();
                    frame.width = frameData.at("width").get<int>();
                    frame.height = frameData.at("height").get<int>();

                    frame.frameTime = frameData.at("frameTime").get<float>();

                    const auto& pivot = frameData.at("pivot");
                    frame.pivot.x = pivot.at("x").get<float>();
                    frame.pivot.y = pivot.at("y").get<float>();

                    anim.frames.push_back(std::move(frame));
                }

                m_Anims.emplace(animName, std::move(anim));
            }

        }
        catch (const json::exception& e)
        {
            logError("Animation parse error: unexpected JSON structure in '{}': {}", fullPath.string(), e.what());

            return;
        }

        // Set current animation on the end iterator
        m_CurrentAnim = m_Anims.end();

        // If there is a default state, switch the animation
        if (m_Anims.contains("default"))
            SetAnimation("default");
    }

    void dae::SpriteComponent::Update(EngineCtx& ctx)
    {
        if (m_CurrentAnim == m_Anims.end())
            return;

        Animation& anim { m_CurrentAnim->second };

        if (anim.isFinished || m_IsPaused)
            return;

        if (anim.accTime < anim.frames[anim.currentFrameId].frameTime)
        {
            anim.accTime += ctx.deltaTime;

            return;
        }

        if (anim.currentFrameId < anim.frames.size() - 1)
        {
            anim.currentFrameId++;
            anim.accTime = 0.0f;

            return;
        }

        anim.currentFrameId = 0;
        anim.accTime = 0.0f;

        anim.isFinished = !anim.isLooping;
    }

    void dae::SpriteComponent::OnDestroy(EngineCtx& ctx)
    {
        ctx.renderer->RemoveRenderable(this);
    }

    int SpriteComponent::GetDrawOrder() const
    {
        return m_DrawOrder;
    }

    void SpriteComponent::Render(const Renderer* renderer) const
    {
        if (m_CurrentAnim == m_Anims.end())
            return;

        const Animation& anim { m_CurrentAnim->second };

        const Frame& frame { anim.frames[anim.currentFrameId] };

        SDL_FRect srcRect {
            static_cast<float>(frame.left),
            static_cast<float>(frame.top),
            static_cast<float>(frame.width),
            static_cast<float>(frame.height) };

        const float width { (m_IsMirrored ? -1.0f : 1.0f) * frame.width * m_Scale };
        const float height { frame.height * m_Scale };

        const glm::vec2 pos { GetOwner()->GetTransform().GetWorldPos() };

        const glm::vec2 dstPos {
            pos.x - width * frame.pivot.x,
            pos.y - height * frame.pivot.y
        };

        SDL_FRect dstRect { dstPos.x, dstPos.y, width, height };

        renderer->RenderTexture(*m_Spritesheet, srcRect, dstRect);
    }

    void SpriteComponent::SetAnimation(const std::string& name)
    {
        auto it { m_Anims.find(name) };

        if (it == m_Anims.end())
        {
            logError("No animation with name '{}' found", name);

            return;
        }

        // Reset current animation
        if (m_CurrentAnim != m_Anims.end())
        {
            Animation& anim { m_CurrentAnim->second };

            anim.isFinished = false;
            anim.currentFrameId = 0;
            anim.accTime = 0.0f;
        }

        // Set new animation
        m_CurrentAnim = it;
    }

    void SpriteComponent::SetAnimationIfChanged(const std::string& name)
    {
        if (m_CurrentAnim != m_Anims.end() && m_CurrentAnim->first == name)
            return;

        SetAnimation(name);
    }

    bool SpriteComponent::IsAnimationFinished(const std::string& name) const
    {
        auto it { m_Anims.find(name) };

        if (it == m_Anims.end())
        {
            logError("No animation with name '{}' found", name);

            return false;
        }

        return it->second.isFinished;
    }

    bool SpriteComponent::GetCurrentFrame(Frame& result)
    {
        if (m_CurrentAnim == m_Anims.end())
            return false;

        const Animation& anim { m_CurrentAnim->second };

        result = anim.frames[anim.currentFrameId];

        return true;
    }
}
#pragma once

#include "GameObject.h"
#include "IRenderable.h"

namespace dae
{
    class Texture2D;

    struct Frame
    {
        int top;
        int left;
        int width;
        int height;

        glm::vec2 pivot;
        float frameTime;
    };

    struct Animation
    {
        // Params
        std::vector<Frame> frames { };
        bool isLooping { };

        // Other
        int currentFrameId { };
        float accTime { };
        bool isFinished { };
    };

    /* Animated texture component */
    class SpriteComponent : public Component, public Registrar<SpriteComponent>, public IRenderable
    {
        public:

        static void Register();

        explicit SpriteComponent(GameObject* owner) : Component(owner) { };
        virtual ~SpriteComponent() = default;

        void OnInit(EngineCtx& ctx) override;
        void Update(EngineCtx& ctx) override;
        void OnDestroy(EngineCtx& ctx) override;

        int GetDrawOrder() const override;
        void Render(const Renderer* renderer) const override;

        void SetAnimation(const std::string& name);
        void SetAnimationIfChanged(const std::string& name);
        bool IsAnimationFinished(const std::string& name) const;

        private:

        using AnimationMap = std::unordered_map<std::string, Animation>;

        // Path to the spritesheet texture file
        std::string m_SpritesheetPath { };
        // Path to the json file containing all the animations
        std::string m_AnimationsPath { };
        // How large the sprite will appear on screen relative to pixel size
        // (scale of 1.0: 1px on screen = 1px in the texture)
        float m_Scale { 1.0f };
        // Draw priority
        int m_DrawOrder { };

        std::shared_ptr<Texture2D> m_Spritesheet { };

        bool m_IsMirrored { };

        AnimationMap m_Anims { };
        AnimationMap::iterator m_CurrentAnim { m_Anims.end() };
    };
}
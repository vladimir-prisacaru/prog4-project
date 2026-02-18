#pragma once

#include "GameObject.h"
#include "Texture2D.h"

namespace dae
{
    class TextureComponent : public Component
    {
        public:

        TextureComponent(GameObject* parent) : Component(parent) { }
        virtual ~TextureComponent() = default;

        void Render() const override;

        void SetTexture(std::shared_ptr<Texture2D> texture);
        void SetTexture(const std::string& file);

        private:

        std::shared_ptr<Texture2D> m_Texture { };
    };
}
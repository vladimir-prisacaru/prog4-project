#pragma once

namespace dae
{
    class Renderer;

    class IRenderable
    {
        public:

        virtual ~IRenderable() = default;

        virtual int GetDrawOrder() const = 0;
        virtual void Render(const Renderer* renderer) const = 0;
    };
}
#pragma once
#include <SDL3/SDL.h>

#include <vector>

#include "Singleton.h"
#include "IRenderable.h"


namespace dae
{
    class Texture2D;

    /* Simple RAII wrapper for the SDL renderer */
    class Renderer final
    {
        public:

        explicit Renderer(SDL_Window* window);

        ~Renderer();
        Renderer(const Renderer& other) = delete;
        Renderer(Renderer&& other) = delete;
        Renderer& operator=(const Renderer& other) = delete;
        Renderer& operator=(Renderer&& other) = delete;

        void AddRenderable(IRenderable* renderable);
        void RemoveRenderable(IRenderable* renderable);

        /* Draws all registered IRenderable objects on the screen */
        void Render();

        /* Call before Render() to create a new frame for ImGui */
        void ImGuiNewFrame();

        void RenderTexture(const Texture2D& texture, const SDL_FRect& dstRect) const;
        void RenderTexture(const Texture2D& texture, const SDL_FRect& srcRect, const SDL_FRect& dstRect) const;
        void RenderTexture(const Texture2D& texture, float x, float y) const;
        void RenderTexture(const Texture2D& texture, float x, float y, float scale) const;
        void RenderTexture(const Texture2D& texture, float x, float y, float width, float height) const;

        SDL_Renderer* GetSDLRenderer() const;

        const SDL_Color& GetBackgroundColor() const { return m_ClearColor; }
        void SetBackgroundColor(const SDL_Color& color) { m_ClearColor = color; }

        private:

        SDL_Renderer* m_Renderer { };
        SDL_Window* m_Window { };
        SDL_Color m_ClearColor { };

        mutable bool m_CreatedNewFrameImGui { };

        std::vector<IRenderable*> m_Renderables { };
    };
}
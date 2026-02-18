#include "Renderer.h"
#include "SceneManager.h"
#include "Texture2D.h"

#include <stdexcept>
#include <cstring>
#include <iostream>



void dae::Renderer::Init(SDL_Window* window)
{
    m_Window = window;

    SDL_SetHint(SDL_HINT_RENDER_VSYNC, "1");



    #if defined(__EMSCRIPTEN__)

    m_renderer = SDL_CreateRenderer(window, nullptr);

    #else

    m_Renderer = SDL_CreateRenderer(window, nullptr);

    #endif // __EMSRIPTEN__



    if (m_Renderer == nullptr)
    {
        std::cout << "Failed to create the renderer: " << SDL_GetError() << "\n";
        throw std::runtime_error(std::string("SDL_CreateRenderer Error: ") + SDL_GetError());
    }
}

void dae::Renderer::Render() const
{
    const auto& color = GetBackgroundColor();
    SDL_SetRenderDrawColor(m_Renderer, color.r, color.g, color.b, color.a);
    SDL_RenderClear(m_Renderer);

    SceneManager::GetInstance().Render();

    SDL_RenderPresent(m_Renderer);
}

void dae::Renderer::Destroy()
{
    if (m_Renderer != nullptr)
    {
        SDL_DestroyRenderer(m_Renderer);
        m_Renderer = nullptr;
    }
}

void dae::Renderer::RenderTexture(const Texture2D& texture, const float x, const float y) const
{
    SDL_FRect dst { };
    dst.x = x;
    dst.y = y;
    SDL_GetTextureSize(texture.GetSDLTexture(), &dst.w, &dst.h);
    SDL_RenderTexture(GetSDLRenderer(), texture.GetSDLTexture(), nullptr, &dst);
}

void dae::Renderer::RenderTexture(const Texture2D& texture, const float x, const float y, const float width, const float height) const
{
    SDL_FRect dst { };
    dst.x = x;
    dst.y = y;
    dst.w = width;
    dst.h = height;
    SDL_RenderTexture(GetSDLRenderer(), texture.GetSDLTexture(), nullptr, &dst);
}

SDL_Renderer* dae::Renderer::GetSDLRenderer() const { return m_Renderer; }
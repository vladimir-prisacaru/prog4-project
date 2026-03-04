#include <imgui.h>
#include <backends/imgui_impl_sdl3.h>
#include <backends/imgui_impl_sdlrenderer3.h>
#include <implot.h>

#include <stdexcept>
#include <cstring>
#include <iostream>

#include "Renderer.h"
#include "SceneManager.h"
#include "Texture2D.h"


void dae::Renderer::Init(SDL_Window* window)
{
    m_Window = window;

    SDL_SetHint(SDL_HINT_RENDER_VSYNC, "1");



    #if defined(__EMSCRIPTEN__)

    m_Renderer = SDL_CreateRenderer(window, nullptr);

    #else

    m_Renderer = SDL_CreateRenderer(window, nullptr);

    #endif // __EMSRIPTEN__



    if (m_Renderer == nullptr)
    {
        std::cout << "Failed to create the renderer: " << SDL_GetError() << "\n";
        throw std::runtime_error(std::string("SDL_CreateRenderer Error: ") + SDL_GetError());
    }


    // Set up ImGui
    ImGui::CreateContext();

    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable; // optional
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable; // optional

    ImGui::StyleColorsDark();

    ImGui_ImplSDL3_InitForSDLRenderer(window, m_Renderer);
    ImGui_ImplSDLRenderer3_Init(m_Renderer);

    // Set up ImPlot
    ImPlot::CreateContext();
}

void dae::Renderer::Render() const
{
    // SDL clear
    const auto& color = GetBackgroundColor();
    SDL_SetRenderDrawColor(m_Renderer, color.r, color.g, color.b, color.a);
    SDL_RenderClear(m_Renderer);

    // Drawing
    SceneManager::GetInstance().Render();

    // ImGui render
    if (m_CreatedNewFrameImGui)
    {
        ImGui::Render();
        ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), m_Renderer);
    }

    // SDL present
    SDL_RenderPresent(m_Renderer);

    m_CreatedNewFrameImGui = false;
}

void dae::Renderer::ImGuiNewFrame()
{
    ImGui_ImplSDLRenderer3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();

    m_CreatedNewFrameImGui = true;
}

void dae::Renderer::Destroy()
{
    // Clean up ImPlot
    ImPlot::DestroyContext();

    // Clean up ImGui
    ImGui_ImplSDLRenderer3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();

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
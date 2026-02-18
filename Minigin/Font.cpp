#include "Font.h"

#include <stdexcept>
#include <SDL3_ttf/SDL_ttf.h>



dae::Font::Font(const std::string& fullPath, float size) : m_Font(nullptr)
{
    m_Font = TTF_OpenFont(fullPath.c_str(), size);

    if (m_Font == nullptr)
    {
        throw std::runtime_error(std::string("Failed to load font: ") + SDL_GetError());
    }
}

dae::Font::~Font()
{
    TTF_CloseFont(m_Font);
}

TTF_Font* dae::Font::GetFont() const
{
    return m_Font;
}
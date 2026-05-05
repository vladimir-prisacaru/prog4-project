#pragma once
#include <glm/vec2.hpp>
#include <string>

struct SDL_Texture;
namespace dae
{
    /* Simple RAII wrapper for an SDL_Texture */
    class Texture2D final
    {
        public:

        explicit Texture2D(SDL_Texture* texture);
        explicit Texture2D(const std::string& fullPath);
        ~Texture2D();

        Texture2D(const Texture2D&) = delete;
        Texture2D(Texture2D&&) = delete;
        Texture2D& operator= (const Texture2D&) = delete;
        Texture2D& operator= (const Texture2D&&) = delete;

        SDL_Texture* GetSDLTexture() const;

        glm::vec2 GetSize() const;

        private:

        SDL_Texture* m_texture;
    };
}

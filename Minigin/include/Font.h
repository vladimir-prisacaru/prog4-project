#pragma once

#include <string>

struct TTF_Font;

namespace dae
{
    /* Simple RAII wrapper for a TTF_Font */
    class Font final
    {
        public:

        explicit Font(const std::string& fullPath, float size);

        ~Font();
        Font(const Font&) = delete;
        Font(Font&&) = delete;
        Font& operator= (const Font&) = delete;
        Font& operator= (const Font&&) = delete;

        TTF_Font* GetFont() const;

        private:

        TTF_Font* m_Font;
    };
}
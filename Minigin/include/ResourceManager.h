#pragma once
#include <filesystem>
#include <string>
#include <memory>
#include <map>
#include "Minigin.h"

namespace dae
{
    class Texture2D;
    class Font;

    class ResourceManager final
    {
        public:

        explicit ResourceManager(const std::filesystem::path& data);

        ~ResourceManager();
        ResourceManager(const ResourceManager& other) = delete;
        ResourceManager(ResourceManager&& other) = delete;
        ResourceManager& operator=(const ResourceManager& other) = delete;
        ResourceManager& operator=(ResourceManager&& other) = delete;

        std::shared_ptr<Texture2D> LoadTexture(const std::string& file, bool pointFilter = false);
        std::shared_ptr<Font> LoadFont(const std::string& file, uint8_t size);

        private:

        friend class Minigin;

        std::filesystem::path m_dataPath;

        void UnloadUnusedResources();

        std::map<std::string, std::shared_ptr<Texture2D>> m_loadedTextures;
        std::map<std::pair<std::string, uint8_t>, std::shared_ptr<Font>> m_loadedFonts;

        EngineCtx m_Ctx { };
    };
}
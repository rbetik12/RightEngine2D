#pragma once

#include "Texture.hpp"
#include <string>
#include <vector>

namespace RightEngine
{
    struct TextureLoaderOptions
    {
    };

//TODO: Implement custom coverter for 3 channel textures

    class TextureLoader
    {
    public:
        TextureLoader(const TextureLoaderOptions& options = TextureLoaderOptions());

        //TODO: Investigate why returning struct here leads to crash
        std::pair<std::vector<uint8_t>, TextureDescriptor> Load(const std::string& path, bool flipVertically = false) const;

        std::shared_ptr<Texture> CreateTexture(const std::string& path, bool flipVertically = false) const;

    private:
        TextureLoaderOptions options;
    };
}

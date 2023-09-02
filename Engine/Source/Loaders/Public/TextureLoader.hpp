#pragma once

#include "Texture.hpp"
#include "AssetLoader.hpp"
#include <string>
#include <vector>

namespace RightEngine
{
    struct TextureLoaderOptions
    {
        TextureType type = TextureType::TEXTURE_2D;
        Format format = Format::NONE;
        bool chooseFormat{ true };
        bool flipVertically{ true };
    };

    class TextureLoader : public AssetLoader
    {
    public:
        AssetHandle Load(const std::string& path,
                         const TextureLoaderOptions& options = {}) const;

        void LoadAsync(AssetHandle& handle,
                       const std::string& path,
                       const TextureLoaderOptions& options = {}) const;

        AssetHandle LoadWithGUID(const std::string& path,
                                 const TextureLoaderOptions& options = {},
                                 const xg::Guid& guid = {}) const;

    private:
        std::pair<std::vector<uint8_t>, TextureDescriptor> LoadTextureData(const std::string& path,
                                                                           const TextureLoaderOptions& options = {}) const;

        AssetHandle _Load(const std::string& path, const TextureLoaderOptions& options, const xg::Guid& guid) const;
    };
}

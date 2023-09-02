#include "TextureLoader.hpp"
#include "Logger.hpp"
#include "Path.hpp"
#include "Assert.hpp"
#include "AssetManager.hpp"
#include "Application.hpp"
#include <Engine/Service/ThreadService.hpp>
#include <stb_image.h>
#include <stb_image_write.h>
#include <fstream>

using namespace RightEngine;

namespace
{
    bool isHDR(const std::string& path)
    {
        const std::string hdr = ".hdr";
        bool isHdr = false;
        isHdr |= stbi_is_hdr(path.c_str());
        isHdr |= std::equal(hdr.rbegin(), hdr.rend(), path.rbegin());
        return isHdr;
    }

    Format ChooseTextureFormat(bool isHdr, int componentsAmount)
    {
        if (isHdr)
        {
            return Format::RGBA32_SFLOAT;
        }

        // TODO: Add check for SRGB support
#if 0
        switch (componentsAmount)
        {
            case 1:
                return Format::R8_UINT;
            case 3:
                return Format::RGB8_UINT;
            case 4:
                return Format::RGBA8_UINT;
            default:
                R_CORE_ASSERT(false, "");
        }
#endif
        switch (componentsAmount)
        {
            case 1:
                return Format::R8_SRGB;
            case 3:
                return Format::RGB8_SRGB;
            case 4:
                return Format::RGBA8_SRGB;
            default:
            R_CORE_ASSERT(false, "");
        }
    }
}

std::pair<std::vector<uint8_t>, TextureDescriptor>TextureLoader::LoadTextureData(const std::string& path,
                                                                                 const TextureLoaderOptions& options) const
{

    std::ifstream file(Path::Absolute(path).c_str(), std::ios::binary | std::ios::ate);
    if (!file.is_open())
    {
        R_CORE_ASSERT(false, "");
    }
    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);
    std::vector<char> fileBuffer(size);
    if (!file.read(fileBuffer.data(), size))
    {
        R_CORE_ASSERT(false, "");
    }

    bool isHdr = isHDR(path);
    if (options.flipVertically)
    {
        stbi_set_flip_vertically_on_load(true);
    }
    else
    {
        stbi_set_flip_vertically_on_load(false);
    }

    TextureDescriptor descriptor;
    int desiredComponents = 0;
    R_CORE_ASSERT(stbi_info_from_memory((stbi_uc*) fileBuffer.data(),
                                        fileBuffer.size(),
                                        &descriptor.width, &descriptor.height, &descriptor.componentAmount), "");
    desiredComponents = descriptor.componentAmount == 3 ? 4 : descriptor.componentAmount;
    if (options.chooseFormat)
    {
        R_CORE_ASSERT(descriptor.width > 0 && descriptor.height > 0 && descriptor.componentAmount > 0, "");
        descriptor.format = ChooseTextureFormat(isHdr, desiredComponents);
    }
    else
    {
        R_CORE_ASSERT(descriptor.format != Format::NONE, "");
        descriptor.format = options.format;
    }

    void* buffer = nullptr;
    if (isHdr)
    {
        buffer = stbi_loadf_from_memory((stbi_uc*)fileBuffer.data(),
                                fileBuffer.size(),
                                &descriptor.width,
                                &descriptor.height,
                                &descriptor.componentAmount,
                                desiredComponents);
    }
    else
    {
        buffer = stbi_load_from_memory((stbi_uc*)fileBuffer.data(),
                                       fileBuffer.size(),
                                       &descriptor.width,
                                       &descriptor.height,
                                       &descriptor.componentAmount,
                                       desiredComponents);
    }

    if (desiredComponents > 0)
    {
        descriptor.componentAmount = desiredComponents;
    }

    if (buffer)
    {
        R_CORE_INFO("Loaded texture at path {0} successfully. {1}x{2} {3} components!", path,
                    descriptor.width,
                    descriptor.height,
                    descriptor.componentAmount);
    }
    else
    {
        R_CORE_ERROR("Failed to load texture at path: {0}", path);
        R_CORE_ASSERT(false, "");
    }

    const size_t textureSize = descriptor.GetTextureSize();
    std::vector<uint8_t> data;
    data.resize(textureSize);
    std::memcpy(data.data(), buffer, textureSize);

    stbi_image_free(buffer);
    return { data, descriptor };
}

AssetHandle TextureLoader::Load(const std::string& path,
                                const TextureLoaderOptions& options) const
{
    return _Load(path, options, xg::Guid());
}

void TextureLoader::LoadAsync(AssetHandle& handle,
                              const std::string& path,
                              const TextureLoaderOptions& options) const
{
    Instance().Service<engine::ThreadService>().AddBackgroundTask([=, &handle]()
        {
            handle = _Load(path, options, xg::Guid());
        });
}

AssetHandle TextureLoader::LoadWithGUID(const std::string& path,
                                        const TextureLoaderOptions& options,
                                        const xg::Guid& guid) const
{
    return _Load(path, options, guid);
}

AssetHandle TextureLoader::_Load(const std::string& path,
                                 const TextureLoaderOptions& options,
                                 const xg::Guid& guid) const
{
    R_CORE_ASSERT(manager, "");

    const auto asset = manager->GetAsset<Texture>(path);
    if (asset)
    {
        return { asset->guid };
    }

    auto [data, descriptor] = LoadTextureData(path, options);
    descriptor.type = options.type;
    auto texture = Device::Get()->CreateTexture(descriptor, data);
    texture->SetSampler(Device::Get()->CreateSampler({}));
    return manager->CacheAsset(texture, path, AssetType::IMAGE, guid);
}

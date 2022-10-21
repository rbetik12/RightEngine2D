#include "TextureLoader.hpp"
#include "Logger.hpp"
#include "Path.hpp"
#include "Assert.hpp"
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
    }
}

std::pair<std::vector<uint8_t>, TextureDescriptor>TextureLoader::Load(const std::string& path,
                                                                      const TextureLoaderOptions& options) const
{
    std::ifstream file(Path::ConvertEnginePathToOSPath(path).c_str(), std::ios::binary | std::ios::ate);
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
    if (options.chooseFormat)
    {
        if (!stbi_info_from_memory((stbi_uc*) fileBuffer.data(), fileBuffer.size(), &descriptor.width, &descriptor.height, &descriptor.componentAmount))
        {
            R_CORE_ASSERT(false, "");
        }
        R_CORE_ASSERT(descriptor.width > 0 && descriptor.height > 0 && descriptor.componentAmount > 0, "");
        desiredComponents = descriptor.componentAmount == 3 ? 4 : descriptor.componentAmount;
        descriptor.format = ChooseTextureFormat(isHdr, desiredComponents);
    }
    else
    {
        R_CORE_ASSERT(descriptor.format != Format::NONE, "");
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

    std::vector<uint8_t> data;
    const size_t textureSize = descriptor.GetTextureSize();
    data.resize(textureSize);
    std::memcpy(data.data(), buffer, textureSize);

    stbi_image_free(buffer);
    return { data, descriptor };
}

std::shared_ptr<Texture> TextureLoader::CreateTexture(const std::string& path,
                                                      TextureType type,
                                                      const TextureLoaderOptions& options) const
{
    auto [data, descriptor] = Load(path, options);
    descriptor.type = type;
    auto texture = Device::Get()->CreateTexture(descriptor, data);
    return texture;
}

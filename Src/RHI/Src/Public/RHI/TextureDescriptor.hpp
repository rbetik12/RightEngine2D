#pragma once

#include <RHI/Config.hpp>
#include <RHI/Assert.hpp>
#include <RHI/Types.hpp>
#include <RHI/Helpers.hpp>

namespace rhi
{

enum class TextureType : uint8_t
{
    NONE,
    TEXTURE_2D,
    TEXTURE_2D_ARRAY,
    TEXTURE_CUBEMAP
};

struct RHI_API TextureDescriptor
{
    uint16_t        m_width = 0;
    uint16_t        m_height = 0;
    uint8_t         m_mipLevels = 1;
    uint8_t         m_layersAmount = 1;
    Format          m_format = Format::NONE;
    TextureType     m_type = TextureType::NONE;

    // Texture size in bytes
    inline uint32_t Size() const
    {
        const uint32_t size = m_width * m_height;
        return PixelSize() * size;
    }

    // One pixel size in bytes
    inline uint8_t PixelSize() const
    {
        const auto components = helpers::FormatComponents(m_format);
        RHI_ASSERT(m_format != Format::NONE && m_width > 0 && m_height > 0 && components > 0);

        switch (m_format)
        {
        case Format::R8_UINT:
        case Format::RGB8_UINT:
        case Format::RGBA8_UINT:
        case Format::RGBA8_UNORM:
            return sizeof(uint8_t) * components;
        case Format::RGB16_SFLOAT:
        case Format::RGBA16_SFLOAT:
            return sizeof(float) / 2 * components;
        case Format::RGB32_SFLOAT:
        case Format::RGBA32_SFLOAT:
            return sizeof(float) * components;
        case Format::R8_SRGB:
        case Format::RGB8_SRGB:
        case Format::RGBA8_SRGB:
            return sizeof(uint8_t) * components;
        case Format::BGRA8_UNORM:
            return sizeof(float) * components;
        default:
            RHI_ASSERT(false);
            return 0;
        }
    }

    inline bool operator==(const TextureDescriptor& otherSpec)
    {
        return m_width == otherSpec.m_width
            && m_height == otherSpec.m_height
            && helpers::FormatComponents(m_format) == helpers::FormatComponents(otherSpec.m_format)
            && m_format == otherSpec.m_format;
    }
};

} // rhi
#pragma once

#include <RHI/Types.hpp>

namespace rhi::helpers
{

inline uint8_t FormatComponents(Format format)
{
    RHI_ASSERT(format != Format::NONE);

    switch (format)
    {
    case Format::R32_SFLOAT: return 1;
    case Format::R32_UINT: return 1;
    case Format::R8_UINT: return 1;
    case Format::RG16_SFLOAT: return 2;
    case Format::RGB8_UINT: return 3;
    case Format::RGBA8_UINT: return 4;
    case Format::RGBA8_UNORM: return 4;
    case Format::RGB16_SFLOAT: return 3;
    case Format::RG32_SFLOAT: return 2;
    case Format::RGB32_SFLOAT: return 3;
    case Format::RGBA16_SFLOAT: return 4;
    case Format::RGBA32_SFLOAT: return 4;
    case Format::RGBA16_UNORM: return 4;
    case Format::RGB16_UNORM: return 3;
    case Format::BGRA8_UNORM: return 4;
    case Format::R8_SRGB: return 1;
    case Format::RGB8_SRGB: return 3;
    case Format::RGBA8_SRGB: return 4;
    case Format::BGRA8_SRGB: return 4;
    case Format::D24_UNORM_S8_UINT: return 1;
    case Format::D32_SFLOAT_S8_UINT: return 1;
    case Format::D32_SFLOAT: return 1;
    }

    RHI_ASSERT(false);
    return 0;
}

inline std::string_view FormatToString(Format format)
{
    switch (format)
    {
    case Format::R8_UINT: return "R8_UINT";
    case Format::RGBA8_UINT: return "RGBA8_UINT";
    case Format::RGBA8_UNORM: return "RGBA8_UNORM";
    case Format::RGBA32_SFLOAT: return "RGBA32_SFLOAT";
    }

    RHI_ASSERT(false);
    return 0;
}

} // rhi::helpers

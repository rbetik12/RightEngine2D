#pragma once

#include <cstdint>

namespace rhi
{

constexpr uint32_t Bit(uint32_t x)
{
    return 1 << x;
}

enum class MemoryType : uint8_t
{
    CPU_ONLY,
    CPU_GPU,
    GPU_ONLY
};

enum class Format : uint8_t
{
    NONE,
    R32_SFLOAT,
    R32_UINT,
    R8_UINT,
    RG16_SFLOAT,
    RGB8_UINT,
    RGBA8_UINT,
    RGBA8_UNORM,
    RGB16_SFLOAT,
    RG32_SFLOAT,
    RGB32_SFLOAT,
    RGBA16_SFLOAT,
    RGBA32_SFLOAT,
    RGBA16_UNORM,
    RGB16_UNORM,
    BGRA8_UNORM,

    //sRGB formats
    R8_SRGB,
    RGB8_SRGB,
    RGBA8_SRGB,
    BGRA8_SRGB,

    //Depth buffer formats
    D24_UNORM_S8_UINT,
    D32_SFLOAT_S8_UINT,
    D32_SFLOAT
};

}
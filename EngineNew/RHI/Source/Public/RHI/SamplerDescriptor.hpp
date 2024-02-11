#pragma once

#include <cstdint>

namespace rhi
{
    enum class SamplerFilter : uint8_t
    {
        NEAREST = 0,
        LINEAR
    };

    enum class AddressMode : uint8_t
    {
        REPEAT = 0,
        CLAMP_TO_EDGE,
        CLAMP_TO_BORDER
    };

	struct SamplerDescriptor
    {
        float                   m_minLod = 0.0f;
        float                   m_maxLod = 1.0f;
        SamplerFilter           m_minFilter = SamplerFilter::NEAREST;
        SamplerFilter           m_magFilter = SamplerFilter::NEAREST;
        SamplerFilter           m_mipMapFilter = SamplerFilter::NEAREST;
        AddressMode             m_addressModeU = AddressMode::REPEAT;
        AddressMode             m_addressModeV = AddressMode::REPEAT;
        AddressMode             m_addressModeW = AddressMode::REPEAT;
        bool                    m_isMipMapped = false;
    };
}

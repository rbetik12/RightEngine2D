#pragma once

#include <RHI/Config.hpp>
#include <RHI/SamplerDescriptor.hpp>

namespace rhi
{
    class RHI_API Sampler
    {
    public:
        virtual ~Sampler() = default;

    protected:
        SamplerDescriptor m_descriptor;

        Sampler(const SamplerDescriptor& desc) : m_descriptor(desc)
        {}
    };
}

#pragma once

#include "Device.hpp"
#include "SamplerDescriptor.hpp"
#include <cstdint>
#include <memory>

namespace RightEngine
{
    class Sampler
    {
    public:
        virtual ~Sampler() = default;

        const SamplerFilter minFilter;
        const SamplerFilter magFilter;
        const SamplerFilter mipMapFilter;

        const bool isMipMapped;

    protected:
        Sampler(const std::shared_ptr<Device>& device, const SamplerDescriptor& descriptor);
    };
}

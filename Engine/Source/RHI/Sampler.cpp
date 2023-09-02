#include "Sampler.hpp"
#include "Renderer.hpp"

using namespace RightEngine;

Sampler::Sampler(const std::shared_ptr<Device>& device, const SamplerDescriptor& descriptor) :
        isMipMapped(descriptor.isMipMapped),
        minFilter(descriptor.minFilter),
        magFilter(descriptor.magFilter),
        mipMapFilter(descriptor.mipMapFilter)
{}

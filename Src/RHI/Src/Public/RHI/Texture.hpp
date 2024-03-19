#pragma once

#include <RHI/Config.hpp>
#include <RHI/TextureDescriptor.hpp>
#include <RHI/Sampler.hpp>

namespace rhi
{
    class RHI_API Texture
    {
    public:
        virtual ~Texture() = default;

        const TextureDescriptor&        Descriptor() const { return m_descriptor; }

        const std::shared_ptr<Sampler>& GetSampler() const { return m_sampler; }

        uint16_t Width() const { return m_descriptor.m_width; }
        uint16_t Height() const { return m_descriptor.m_height; }

    protected:
        TextureDescriptor           m_descriptor;
        std::shared_ptr<Sampler>    m_sampler;

        Texture(const TextureDescriptor& desc, const std::shared_ptr<Sampler>& sampler) : m_descriptor(desc), m_sampler(sampler)
        {}
    };
}

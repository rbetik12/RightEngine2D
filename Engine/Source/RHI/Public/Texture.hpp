#pragma once

#include "Assert.hpp"
#include "AssetBase.hpp"
#include "Sampler.hpp"
#include "Device.hpp"
#include "TextureDescriptor.hpp"
#include <cstdint>
#include <string>
#include <memory>

namespace RightEngine
{
    class Texture : public AssetBase
    {
    public:
        ASSET_BASE()

        virtual ~Texture() = default;

        inline int GetWidth() const
        { return specification.width; }

        inline int GetHeight() const
        { return specification.height; }

        inline uint32_t GetId() const
        { return id; }

        inline std::shared_ptr<Sampler> GetSampler() const
        { return sampler; }

        void SetSampler(const std::shared_ptr<Sampler>& aSampler)
        {
//            R_CORE_ASSERT(ValidateSampler(aSampler), "");
            sampler = aSampler;
        }

        inline const TextureDescriptor& GetSpecification() const
        { return specification; }

        virtual void CopyFrom(const std::shared_ptr<Texture>& texture, const TextureCopy& srcCopy, const TextureCopy& dstCopy) = 0;

        virtual void* GetNativeHandle() const = 0;

        virtual std::shared_ptr<Buffer> Data() = 0;

    protected:
        Texture(const std::shared_ptr<Device>& device,
                const TextureDescriptor& aSpecification,
                const std::vector<uint8_t>& data) : specification(aSpecification)
        {}

        virtual bool ValidateSampler(const std::shared_ptr<Sampler>& sampler) const = 0;

        uint32_t id;
        TextureDescriptor specification;
        std::shared_ptr<Sampler> sampler;
    };
}
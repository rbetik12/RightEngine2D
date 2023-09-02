#pragma once

#include "Texture.hpp"
#include "VulkanDevice.hpp"
#include "VulkanCommandBuffer.hpp"

namespace RightEngine
{
    class VulkanTexture : public Texture
    {
    public:
        VulkanTexture(const std::shared_ptr<Device>& device,
                const TextureDescriptor& aSpecification,
                const std::vector<uint8_t>& data);

        virtual ~VulkanTexture() override;

        virtual void* GetNativeHandle() const override;

        virtual void CopyFrom(const std::shared_ptr<Texture>& texture, const TextureCopy& srcCopy, const TextureCopy& dstCopy) override;

        virtual std::shared_ptr<Buffer> Data() override;

        VkImageView GetImageView() const
        { return textureImageView; }

        VkImage GetImage() const
        { return textureImage; }

        static void ChangeImageLayout(VkImage image, 
            VkImageLayout oldLayout, 
            VkImageLayout newLayout,
            Format format,
            int layers, 
            int mipmaps, 
            bool isDepth = false);

    protected:
        void Init(const std::shared_ptr<VulkanDevice>& device, const std::vector<uint8_t>& data);

        virtual bool ValidateSampler(const std::shared_ptr<Sampler>& sampler) const override;

        std::shared_ptr<Buffer> stagingBuffer;
        VkImage textureImage;
        VkImageView textureImageView;
        VmaAllocation allocation;
    };
}

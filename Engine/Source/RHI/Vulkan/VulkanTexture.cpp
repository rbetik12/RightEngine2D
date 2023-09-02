#include "VulkanTexture.hpp"
#include "VulkanConverters.hpp"
#include "VulkanUtils.hpp"
#include "VulkanBuffer.hpp"
#include "Buffer.hpp"
#include <vk-tools/VulkanTools.h>

using namespace RightEngine;
namespace
{
    void CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height)
    {
        CommandBufferDescriptor commandBufferDescriptor;
        commandBufferDescriptor.type = CommandBufferType::GRAPHICS;
        auto commandBuffer = std::static_pointer_cast<VulkanCommandBuffer>(
                Device::Get()->CreateCommandBuffer(commandBufferDescriptor));

        VulkanUtils::BeginCommandBuffer(commandBuffer, true);

        VkBufferImageCopy region{};
        region.bufferOffset = 0;
        region.bufferRowLength = 0;
        region.bufferImageHeight = 0;

        region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        region.imageSubresource.mipLevel = 0;
        region.imageSubresource.baseArrayLayer = 0;
        region.imageSubresource.layerCount = 1;

        region.imageOffset = { 0, 0, 0 };
        region.imageExtent = { width, height, 1 };

        commandBuffer->Enqueue([=](auto cmdBuffer)
                               {
                                   vkCmdCopyBufferToImage(
                                           VK_CMD(cmdBuffer)->GetBuffer(),
                                           buffer,
                                           image,
                                           VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                           1,
                                           &region
                                   );
                               });

        VulkanUtils::EndCommandBuffer(VK_DEVICE(), commandBuffer);
    }

    void CopyImageToBuffer(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height)
    {
        CommandBufferDescriptor commandBufferDescriptor;
        commandBufferDescriptor.type = CommandBufferType::GRAPHICS;
        auto commandBuffer = std::static_pointer_cast<VulkanCommandBuffer>(
            Device::Get()->CreateCommandBuffer(commandBufferDescriptor));

        VulkanUtils::BeginCommandBuffer(commandBuffer, true);

        VkBufferImageCopy region{};
        region.bufferOffset = 0;
        region.bufferRowLength = 0;
        region.bufferImageHeight = 0;

        region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        region.imageSubresource.mipLevel = 0;
        region.imageSubresource.baseArrayLayer = 0;
        region.imageSubresource.layerCount = 1;

        region.imageOffset = { 0, 0, 0 };
        region.imageExtent = { width, height, 1 };

        commandBuffer->Enqueue([=](auto cmdBuffer)
            {
                vkCmdCopyImageToBuffer(
                    VK_CMD(cmdBuffer)->GetBuffer(),
                    image,
                    VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                    buffer,
                    1,
                    &region
                );
            });

        VulkanUtils::EndCommandBuffer(VK_DEVICE(), commandBuffer);
    }

    bool IsDepthTexture(Format format)
    {
        if (format == Format::D24_UNORM_S8_UINT || format == Format::D32_SFLOAT_S8_UINT || format == Format::D32_SFLOAT)
        {
            return true;
        }
        return false;
    }
}

VulkanTexture::VulkanTexture(const std::shared_ptr<Device>& device,
                             const TextureDescriptor& aSpecification,
                             const std::vector<uint8_t>& data) : Texture(device, aSpecification, data)
{
    const auto vkDevice = std::static_pointer_cast<VulkanDevice>(device);
    Init(vkDevice, data);
}

void VulkanTexture::Init(const std::shared_ptr<VulkanDevice>& device,
                         const std::vector<uint8_t>& data)
{
    if (!data.empty())
    {
        BufferDescriptor stagingBufferDesc;
        stagingBufferDesc.size = specification.GetTextureSize();
        stagingBufferDesc.type = BufferType::TRANSFER_SRC;
        stagingBufferDesc.memoryType = MemoryType::CPU_ONLY;
        stagingBuffer = device->CreateBuffer(stagingBufferDesc, nullptr);
        auto ptr = stagingBuffer->Map();
        memcpy(ptr, data.data(), stagingBufferDesc.size);
        stagingBuffer->UnMap();
    }

    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VulkanConverters::ImageType(specification.type);
    imageInfo.extent.width = specification.width;
    imageInfo.extent.height = specification.height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = specification.mipLevels;
    imageInfo.arrayLayers = specification.type == TextureType::CUBEMAP ? 6 : 1;
    imageInfo.format = VulkanConverters::Format(specification.format);
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    if (IsDepthTexture(specification.format))
    {
        imageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    }
    else
    {
        // TODO: Make proper validation of image usage with
        // https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/vkGetPhysicalDeviceImageFormatProperties.html

        if (specification.format == Format::R8_SRGB)
        {
            imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT
                              | VK_IMAGE_USAGE_SAMPLED_BIT
                              | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
        }
        else
        {
            imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT
                              | VK_IMAGE_USAGE_SAMPLED_BIT
                              | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT
                              | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
        }
    }
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.flags = specification.type == TextureType::CUBEMAP ? VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT : 0;

    VmaAllocationCreateInfo imageAllocInfo = {};
    imageAllocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
    imageAllocInfo.requiredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

    vmaCreateImage(VK_DEVICE()->GetAllocator(), &imageInfo, &imageAllocInfo, &textureImage, &allocation, nullptr);

    if (!IsDepthTexture(specification.format))
    {
		ChangeImageLayout(textureImage,
                          VK_IMAGE_LAYOUT_UNDEFINED,
                          VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                          specification.format,
                          specification.type == TextureType::CUBEMAP ? 6 : 1,
                          specification.mipLevels);
        if (!data.empty())
        {
            CopyBufferToImage(std::static_pointer_cast<VulkanBuffer>(stagingBuffer)->GetBuffer(),
                              textureImage,
                              specification.width,
                              specification.height);

            stagingBuffer.reset();
        }
        ChangeImageLayout(textureImage,
                              VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                              VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                              specification.format,
                              specification.type == TextureType::CUBEMAP ? 6 : 1,
                              specification.mipLevels);
    }
    else
    {
        ChangeImageLayout(textureImage,
            VK_IMAGE_LAYOUT_UNDEFINED,
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, specification.format,
            specification.type == TextureType::CUBEMAP ? 6 : 1,
            specification.mipLevels,
            true);
    }

    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = textureImage;
    viewInfo.viewType = specification.type == TextureType::CUBEMAP ? VK_IMAGE_VIEW_TYPE_CUBE : VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = VulkanConverters::Format(specification.format);
    if (IsDepthTexture(specification.format))
    {
	    switch (specification.format)
	    {
			case Format::D32_SFLOAT_S8_UINT:
			{
				viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
                break;
			}
			case Format::D32_SFLOAT:
			{
                viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
                break;
			}
	    }
    }
    else
    {
        viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    }
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = specification.mipLevels;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = specification.type == TextureType::CUBEMAP ? 6 : 1;

    if (vkCreateImageView(device->GetDevice(), &viewInfo, nullptr, &textureImageView) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create texture image view!");
    }
}

VulkanTexture::~VulkanTexture()
{
    vkDestroyImageView(VK_DEVICE()->GetDevice(), textureImageView, nullptr);
    vkDestroyImage(VK_DEVICE()->GetDevice(), textureImage, nullptr);
    vmaFreeMemory(VK_DEVICE()->GetAllocator(), allocation);
}

void VulkanTexture::ChangeImageLayout(VkImage image, 
    VkImageLayout oldLayout, 
    VkImageLayout newLayout,
    Format format,
    int layers, 
    int mipmaps, 
    bool isDepth)
{
    CommandBufferDescriptor commandBufferDescriptor;
    commandBufferDescriptor.type = CommandBufferType::GRAPHICS;
    auto commandBuffer = std::static_pointer_cast<VulkanCommandBuffer>(
        Device::Get()->CreateCommandBuffer(commandBufferDescriptor));

    VulkanUtils::BeginCommandBuffer(commandBuffer, true);
    VkImageSubresourceRange srcSubRange = {};
    if (isDepth)
    {
        switch (format)
        {
        case Format::D32_SFLOAT_S8_UINT:
        {
            srcSubRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
            break;
        }
        case Format::D32_SFLOAT:
        {
            srcSubRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
            break;
        }
        default:
            R_CORE_ASSERT(false, "");
        }
    }
    else
    {
        srcSubRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    }
    srcSubRange.baseMipLevel = 0;
    srcSubRange.levelCount = mipmaps;
    srcSubRange.layerCount = layers;
    vks::tools::setImageLayout(commandBuffer->GetBuffer(),
        image,
        oldLayout,
        newLayout,
        srcSubRange);

    VulkanUtils::EndCommandBuffer(VK_DEVICE(), commandBuffer);
}

void VulkanTexture::CopyFrom(const std::shared_ptr<Texture>& texture,
                             const TextureCopy& srcCopy,
                             const TextureCopy& dstCopy)
{
    R_CORE_ASSERT(sampler && texture && texture->GetSampler(), "");
//TODO: Add smarter check
    R_CORE_ASSERT(specification.width / glm::exp2(dstCopy.mipLevel) == texture->GetSpecification().width / glm::exp2(srcCopy.mipLevel)
                  && specification.height / glm::exp2(dstCopy.mipLevel)  == texture->GetSpecification().height / glm::exp2(srcCopy.mipLevel), "");
    const auto srcTexture = std::static_pointer_cast<VulkanTexture>(texture);
    const auto copyCmdBuffer = std::static_pointer_cast<VulkanCommandBuffer>(
            Device::Get()->CreateCommandBuffer({ CommandBufferType::GRAPHICS }));
    VulkanUtils::BeginCommandBuffer(copyCmdBuffer, true);
    VkImageSubresourceRange dstSubRange = {};
    dstSubRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    dstSubRange.baseMipLevel = 0;
    dstSubRange.levelCount = specification.mipLevels;
    dstSubRange.layerCount = specification.type == TextureType::CUBEMAP ? 6 : 1;
    vks::tools::setImageLayout(copyCmdBuffer->GetBuffer(),
                               textureImage,
                               VulkanConverters::TextureUsage(dstCopy.usage),
                               VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                               dstSubRange);

    VkImageSubresourceRange srcSubRange = {};
    srcSubRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    srcSubRange.baseMipLevel = 0;
    srcSubRange.levelCount = srcTexture->GetSpecification().mipLevels;
    srcSubRange.layerCount = srcTexture->GetSpecification().type == TextureType::CUBEMAP ? 6 : 1;
    vks::tools::setImageLayout(copyCmdBuffer->GetBuffer(),
                               srcTexture->GetImage(),
                               VulkanConverters::TextureUsage(srcCopy.usage),
                               VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                               srcSubRange);

    VkImageCopy copyRegion{};
    copyRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    copyRegion.srcSubresource.baseArrayLayer = srcCopy.layerNum;
    copyRegion.srcSubresource.mipLevel = srcCopy.mipLevel;
    copyRegion.srcSubresource.layerCount = 1;
    copyRegion.srcOffset = { 0, 0, 0 };

    copyRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    copyRegion.dstSubresource.baseArrayLayer = dstCopy.layerNum;
    copyRegion.dstSubresource.mipLevel = dstCopy.mipLevel;
    copyRegion.dstSubresource.layerCount = 1;
    copyRegion.dstOffset = { 0, 0, 0 };

    copyRegion.extent.width = static_cast<uint32_t>(texture->GetSpecification().width);
    copyRegion.extent.height = static_cast<uint32_t>(texture->GetSpecification().height);
    copyRegion.extent.depth = 1;

    vkCmdCopyImage(
            copyCmdBuffer->GetBuffer(),
            srcTexture->GetImage(),
            VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
            textureImage,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            1,
            &copyRegion);

    vks::tools::setImageLayout(copyCmdBuffer->GetBuffer(),
                               textureImage,
                               VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                               VulkanConverters::TextureUsage(dstCopy.usage),
                               dstSubRange);

    vks::tools::setImageLayout(copyCmdBuffer->GetBuffer(),
                               srcTexture->GetImage(),
                               VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                               VulkanConverters::TextureUsage(srcCopy.usage),
                               srcSubRange);

    VulkanUtils::EndCommandBuffer(VK_DEVICE(), copyCmdBuffer);
}

std::shared_ptr<Buffer> VulkanTexture::Data()
{
    BufferDescriptor bufferDesc;
    bufferDesc.size = specification.GetTextureSize();
    bufferDesc.type = BufferType::TRANSFER_DST;
    bufferDesc.memoryType = MemoryType::CPU_ONLY;
    auto buffer = VK_DEVICE()->CreateBuffer(bufferDesc, nullptr);

    ChangeImageLayout(textureImage,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
        specification.format,
        specification.type == TextureType::CUBEMAP ? 6 : 1,
        specification.mipLevels);
    CopyImageToBuffer(std::static_pointer_cast<VulkanBuffer>(buffer)->GetBuffer(),
        textureImage,
        specification.width,
        specification.height);
    ChangeImageLayout(textureImage,
        VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        specification.format,
        specification.type == TextureType::CUBEMAP ? 6 : 1,
        specification.mipLevels);
    return buffer;
}

bool VulkanTexture::ValidateSampler(const std::shared_ptr<Sampler>& sampler) const
{
    bool result = false;
    result |= vks::tools::formatIsFilterable(VK_DEVICE()->GetPhysicalDevice(), VulkanConverters::Format(specification.format), VK_IMAGE_TILING_LINEAR);

    return result;
}

void* VulkanTexture::GetNativeHandle() const
{
    return textureImage;
}

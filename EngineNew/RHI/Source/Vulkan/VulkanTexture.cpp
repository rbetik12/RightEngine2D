#include "VulkanTexture.hpp"
#include "VulkanBuffer.hpp"
#include "VulkanDevice.hpp"
#include "VulkanHelpers.hpp"
#include <vk-tools/VulkanTools.h>

namespace rhi::vulkan
{

namespace
{

inline bool IsDepthTexture(Format format)
{
    if (format == Format::D24_UNORM_S8_UINT || 
        format == Format::D32_SFLOAT_S8_UINT || 
        format == Format::D32_SFLOAT)
    {
        return true;
    }
    return false;
}

void CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height)
{
    CommandBuffer cmdBuffer;
    cmdBuffer.Begin();

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

    cmdBuffer.Push([&](VkCommandBuffer cmdBufferPtr)
        {
            vkCmdCopyBufferToImage(
                cmdBufferPtr,
                buffer,
                image,
                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                1,
                &region
            );
        });

    cmdBuffer.End();
}

} // namespace unnamed

VulkanTexture::VulkanTexture(const TextureDescriptor& desc, const void* data) : Texture(desc), m_layout(VK_IMAGE_LAYOUT_UNDEFINED)
{
    RHI_ASSERT(desc.m_mipLevels > 0);

    if (data)
    {
        BufferDescriptor stagingBufferDesc;
        stagingBufferDesc.m_size = desc.Size();
        stagingBufferDesc.m_type = BufferType::TRANSFER_SRC;
        stagingBufferDesc.m_memoryType = MemoryType::CPU_ONLY;
		m_stagingBuffer = VulkanDevice::s_ctx.m_instance->CreateBuffer(stagingBufferDesc, nullptr);
        auto ptr = m_stagingBuffer->Map();
        memcpy(ptr, data, stagingBufferDesc.m_size);
        m_stagingBuffer->UnMap();
    }

    VkImageCreateInfo imageCreateInfo{};
    imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    // TODO: Currently we support only 2D textures
    imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
    imageCreateInfo.extent.width = desc.m_width;
    imageCreateInfo.extent.height = desc.m_height;
    imageCreateInfo.extent.depth = 1;
    imageCreateInfo.mipLevels = desc.m_mipLevels;
    imageCreateInfo.arrayLayers = desc.m_layersAmount;
    imageCreateInfo.format = helpers::Format(desc.m_format);
    imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    // TODO: Make proper validation of image usage with
    // https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/vkGetPhysicalDeviceImageFormatProperties.html
    // Probably we don't need it, because all of that would be validated in validation layers anyway...

    if (IsDepthTexture(desc.m_format))
    {
        imageCreateInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    }
    else if (desc.m_format == Format::R8_SRGB)
    {
        imageCreateInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT
            | VK_IMAGE_USAGE_SAMPLED_BIT
            | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    }
    else
    {
        imageCreateInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT
            | VK_IMAGE_USAGE_SAMPLED_BIT
            | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT
            | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    }

    imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;

    switch (desc.m_type)
    {
        case TextureType::TEXTURE_2D_ARRAY:
        {
			imageCreateInfo.flags = VK_IMAGE_CREATE_2D_ARRAY_COMPATIBLE_BIT;
            break;
        }
        case TextureType::TEXTURE_CUBEMAP:
        {
            imageCreateInfo.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
            break;
        }
    default: 
        break;
    }

    VmaAllocationCreateInfo imageAllocInfo = {};
    imageAllocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
    imageAllocInfo.requiredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

    const auto status = vmaCreateImage(VulkanDevice::s_ctx.m_allocator, &imageCreateInfo, &imageAllocInfo, &m_image, &m_allocation, nullptr);

    RHI_ASSERT(status == VK_SUCCESS);

    // TODO: Add later support for more layers

    if (!IsDepthTexture(m_descriptor.m_format))
    {
        ChangeImageLayout(m_image,
            VK_IMAGE_LAYOUT_UNDEFINED,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            m_descriptor.m_format,
            m_descriptor.m_type == TextureType::TEXTURE_CUBEMAP ? 6 : 1,
            m_descriptor.m_mipLevels);
        if (data)
        {
            CopyBufferToImage(std::static_pointer_cast<VulkanBuffer>(m_stagingBuffer)->Raw(),
                m_image,
                m_descriptor.m_width,
                m_descriptor.m_height);

            m_stagingBuffer.reset();
        }
        ChangeImageLayout(m_image,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            m_descriptor.m_format,
            m_descriptor.m_type == TextureType::TEXTURE_CUBEMAP ? 6 : 1,
            m_descriptor.m_mipLevels);
    }
    else
    {
        ChangeImageLayout(m_image,
            VK_IMAGE_LAYOUT_UNDEFINED,
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 
            m_descriptor.m_format,
            m_descriptor.m_type == TextureType::TEXTURE_CUBEMAP ? 6 : 1,
            m_descriptor.m_mipLevels,
            true);
    }

    // TODO: For texture arrays we must create as many image views as we have layers in the image

    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = m_image;
    viewInfo.viewType = m_descriptor.m_type == TextureType::TEXTURE_CUBEMAP ? VK_IMAGE_VIEW_TYPE_CUBE : VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = helpers::Format(m_descriptor.m_format);
    if (IsDepthTexture(m_descriptor.m_format))
    {
        switch (m_descriptor.m_format)
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
    viewInfo.subresourceRange.levelCount = m_descriptor.m_mipLevels;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = m_descriptor.m_type == TextureType::TEXTURE_CUBEMAP ? 6 : 1;

    auto& view = m_imageViews.emplace_back();
    RHI_ASSERT(vkCreateImageView(VulkanDevice::s_ctx.m_device, &viewInfo, nullptr, &view) == VK_SUCCESS);
    RHI_ASSERT(view);
}

VulkanTexture::~VulkanTexture()
{
    // TODO: Currently we support only one image view, change later!
    vkDestroyImageView(VulkanDevice::s_ctx.m_device, m_imageViews[0], nullptr);
    vmaDestroyImage(VulkanDevice::s_ctx.m_allocator, m_image, m_allocation);
}

void VulkanTexture::ChangeImageLayout(VkCommandBuffer cmdBuffer, VkImageLayout oldLayout, VkImageLayout newLayout)
{
    const bool isDepth = m_descriptor.m_format == Format::D32_SFLOAT
	|| m_descriptor.m_format == Format::D32_SFLOAT_S8_UINT;

    ChangeImageLayout(cmdBuffer,
        m_image, 
        oldLayout, 
        newLayout, 
        m_descriptor.m_format, 
        m_descriptor.m_layersAmount,
        m_descriptor.m_mipLevels,
        isDepth);
}

void VulkanTexture::ChangeImageLayout(VkImageLayout oldLayout, VkImageLayout newLayout)
{
    const bool isDepth = m_descriptor.m_format == Format::D32_SFLOAT
        || m_descriptor.m_format == Format::D32_SFLOAT_S8_UINT;

    ChangeImageLayout(m_image,
        oldLayout,
        newLayout,
        m_descriptor.m_format,
        m_descriptor.m_layersAmount,
        m_descriptor.m_mipLevels,
        isDepth);
}

void VulkanTexture::ChangeImageLayout(VkCommandBuffer cmdBuffer,
                                      VkImage image,
                                      VkImageLayout oldLayout, 
                                      VkImageLayout newLayout,
                                      Format format, 
                                      int layers, 
                                      int mipmaps, 
                                      bool isDepth)
{
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
            RHI_ASSERT(false);
        }
    }
    else
    {
        srcSubRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    }
    srcSubRange.baseMipLevel = 0;
    srcSubRange.levelCount = mipmaps;
    srcSubRange.layerCount = layers;

    vks::tools::setImageLayout(cmdBuffer,
        image,
        oldLayout,
        newLayout,
        srcSubRange);

    m_layout = newLayout;
}

void VulkanTexture::ChangeImageLayout(VkImage image, 
									    VkImageLayout oldLayout,
									    VkImageLayout newLayout, 
									    Format format,
										int layers, 
									    int mipmaps, 
									    bool isDepth)
{
    CommandBuffer cmdBuffer;
    cmdBuffer.Begin();

    ChangeImageLayout(cmdBuffer.Raw(),
        m_image,
        oldLayout,
        newLayout,
        m_descriptor.m_format,
        m_descriptor.m_layersAmount,
        m_descriptor.m_mipLevels,
        isDepth);

    cmdBuffer.End();

    VulkanDevice::s_ctx.m_instance->Execute(cmdBuffer)->Wait();
}

} // namespace rhi::vulkan

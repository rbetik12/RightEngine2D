#include "VulkanTexture.hpp"
#include "VulkanDevice.hpp"
#include "VulkanHelpers.hpp"

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
    }

	VulkanTexture::VulkanTexture(const TextureDescriptor& desc, const eastl::vector<uint8_t>& data) : Texture(desc), m_layout(VK_IMAGE_LAYOUT_UNDEFINED)
	{
        if (!data.empty())
        {
            BufferDescriptor stagingBufferDesc;
            stagingBufferDesc.m_size = desc.Size();
            stagingBufferDesc.m_type = BufferType::TRANSFER_SRC;
            stagingBufferDesc.m_memoryType = MemoryType::CPU_ONLY;
			m_stagingBuffer = VulkanDevice::s_ctx.m_instance->CreateBuffer(stagingBufferDesc, nullptr);
            auto ptr = m_stagingBuffer->Map();
            memcpy(ptr, data.data(), stagingBufferDesc.m_size);
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

        vmaCreateImage(VulkanDevice::s_ctx.m_allocator, &imageCreateInfo, &imageAllocInfo, &m_image, &m_allocation, nullptr);
	}

	VulkanTexture::~VulkanTexture()
	{
	}
}

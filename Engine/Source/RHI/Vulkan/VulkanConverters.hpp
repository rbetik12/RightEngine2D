#pragma once

#include "Assert.hpp"
#include "Types.hpp"
#include <glm/vec2.hpp>
#include <vulkan/vulkan.h>

namespace RightEngine
{
    class VulkanConverters
    {
    public:
        inline static VkPresentModeKHR PresentMode(PresentMode mode)
        {
            switch (mode)
            {
                case PresentMode::IMMEDIATE:
                    return VK_PRESENT_MODE_IMMEDIATE_KHR;
                case PresentMode::FIFO:
                    return VK_PRESENT_MODE_FIFO_KHR;
                case PresentMode::MAILBOX:
                    return VK_PRESENT_MODE_MAILBOX_KHR;
            }
        }

        inline static VkExtent2D Extent(const glm::ivec2& extent)
        {
            return { static_cast<uint32_t>(extent.x), static_cast<uint32_t>(extent.y) };
        }

        inline static VkFormat Format(Format format)
        {
            switch (format)
            {
                case Format::RGBA8_SRGB:
                    return VK_FORMAT_R8G8B8A8_SRGB;
                case Format::BGRA8_SRGB:
                    return VK_FORMAT_B8G8R8A8_SRGB;
                case Format::R32_SFLOAT:
                    return VK_FORMAT_R32_SFLOAT;
                case Format::RG32_SFLOAT:
                    return VK_FORMAT_R32G32_SFLOAT;
                case Format::RGB32_SFLOAT:
                    return VK_FORMAT_R32G32B32_SFLOAT;
                case Format::RGBA32_SFLOAT:
                    return VK_FORMAT_R32G32B32A32_SFLOAT;
                case Format::R32_UINT:
                    return VK_FORMAT_R32_UINT;
                case Format::R8_UINT:
                    return VK_FORMAT_R8_UINT;
                case Format::RGBA8_UINT:
                    return VK_FORMAT_R8G8B8A8_UINT;
                case Format::D24_UNORM_S8_UINT:
                    return VK_FORMAT_D24_UNORM_S8_UINT;
                case Format::RGB16_SFLOAT:
                    return VK_FORMAT_R16G16B16_SFLOAT;
                case Format::RGB16_UNORM:
                    return VK_FORMAT_R16G16B16_UNORM;
                case Format::RGBA16_SFLOAT:
                    return VK_FORMAT_R16G16B16A16_SFLOAT;
                case Format::RGB8_UINT:
                    return VK_FORMAT_R8G8B8_UINT;
                case Format::D32_SFLOAT_S8_UINT:
                    return VK_FORMAT_D32_SFLOAT_S8_UINT;
                case Format::RG16_SFLOAT:
                    return VK_FORMAT_R16G16_SFLOAT;
                case Format::RGBA16_UNORM:
                    return VK_FORMAT_R16G16B16A16_UNORM;
                case Format::BGRA8_UNORM:
                    return VK_FORMAT_B8G8R8A8_UNORM;
                case Format::R8_SRGB:
                    return VK_FORMAT_R8_SRGB;
                case Format::RGB8_SRGB:
                    return VK_FORMAT_R8G8B8_SRGB;
				case Format::D32_SFLOAT:
                    return VK_FORMAT_D32_SFLOAT;
                default:
                    R_CORE_ASSERT(false, "");
            }
        }

        inline static VkBufferUsageFlags BufferUsage(BufferType type)
        {
            switch (type)
            {
                case BufferType::VERTEX:
                    return VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
                case BufferType::INDEX:
                    return VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
                case BufferType::UNIFORM:
                    return VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
                case BufferType::TRANSFER_DST:
                    return VK_BUFFER_USAGE_TRANSFER_DST_BIT;
                case BufferType::TRANSFER_SRC:
                    return VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
                default:
                R_CORE_ASSERT(false, "");
            }
        }
/*
        inline static VkMemoryPropertyFlags MemoryProperty(MemoryType type)
        {
            VkMemoryPropertyFlags flags = 0;
            if (type & MEMORY_TYPE_DEVICE_LOCAL)
            {
                flags |= VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
            }
            if (type & MEMORY_TYPE_HOST_VISIBLE)
            {
                flags |= VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
            }
            if (type & MEMORY_TYPE_HOST_COHERENT)
            {
                flags |= VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
            }

            return flags;
        }
*/

        inline static VkShaderStageFlags ShaderStage(ShaderStage stage)
        {
            switch (stage)
            {
                case ShaderType::VERTEX:
                    return VK_SHADER_STAGE_VERTEX_BIT;
                case ShaderType::FRAGMENT:
                    return VK_SHADER_STAGE_FRAGMENT_BIT;
                default:
                R_CORE_ASSERT(false, "");
            }
        }

        inline static VkImageType ImageType(TextureType type)
        {
            switch (type)
            {
                case TextureType::CUBEMAP:
                    return VK_IMAGE_TYPE_2D;
                case TextureType::TEXTURE_2D:
                    return VK_IMAGE_TYPE_2D;
                default:
                R_CORE_ASSERT(false, "");
            }
        }

        inline static VkAttachmentLoadOp LoadOperation(AttachmentLoadOperation loadOp)
        {
            switch (loadOp)
            {
                case AttachmentLoadOperation::UNDEFINED:
                    return VK_ATTACHMENT_LOAD_OP_DONT_CARE;
                case AttachmentLoadOperation::LOAD:
                    return VK_ATTACHMENT_LOAD_OP_LOAD;
                case AttachmentLoadOperation::CLEAR:
                    return VK_ATTACHMENT_LOAD_OP_CLEAR;
                default:
                R_CORE_ASSERT(false, "");
            }
        }

        inline static VkAttachmentStoreOp StoreOperation(AttachmentStoreOperation storeOp)
        {
            switch (storeOp)
            {
                case AttachmentStoreOperation::UNDEFINED:
                    return VK_ATTACHMENT_STORE_OP_DONT_CARE;
                case AttachmentStoreOperation::STORE:
                    return VK_ATTACHMENT_STORE_OP_STORE;
            }
        }

        inline static VkImageLayout TextureUsage(TextureUsage usage)
        {
            switch (usage)
            {
                case COLOR_ATTACHMENT:
                    return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
                case DEPTH_STENCIL_ATTACHMENT:
                    return VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
                case SHADER_READ_ONLY:
                    return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                case TRANSFER_DST:
                    return VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
                case TRANSFER_SRC:
                    return VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
                default:
                    R_CORE_ASSERT(false, "");
                    return VK_IMAGE_LAYOUT_UNDEFINED;
            }
        }

        inline static VmaMemoryUsage MemoryUsage(MemoryType type)
        {
            switch (type)
            {
                case MemoryType::CPU_ONLY:
                    return VMA_MEMORY_USAGE_CPU_ONLY;
                case MemoryType::CPU_GPU:
                    return VMA_MEMORY_USAGE_CPU_TO_GPU;
                case MemoryType::GPU_ONLY:
                    return VMA_MEMORY_USAGE_GPU_ONLY;
                default:
                R_CORE_ASSERT(false, "")
            }
        }

        inline static VkCompareOp CompareOp(CompareOp op)
        {
            switch (op)
            {
                case CompareOp::LESS:
                    return VK_COMPARE_OP_LESS;
                case CompareOp::LESS_OR_EQUAL:
                    return VK_COMPARE_OP_LESS_OR_EQUAL;
                case CompareOp::GREATER:
                    return VK_COMPARE_OP_GREATER;
                default:
                    R_CORE_ASSERT(false, "");
            }
        }

        inline static VkCullModeFlags CullMode(CullMode mode)
        {
            switch (mode)
            {
                case CullMode::FRONT:
                    return VK_CULL_MODE_FRONT_BIT;
                case CullMode::BACK:
                    return VK_CULL_MODE_BACK_BIT;
                default:
                    R_CORE_ASSERT(false, "");
            }
        }

        inline static VkSamplerAddressMode AddressMode(AddressMode mode)
        {
	        switch (mode)
	        {
	        case AddressMode::Repeat: 
                return VK_SAMPLER_ADDRESS_MODE_REPEAT;
	        case AddressMode::ClampToEdge: 
                return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	        case AddressMode::ClampToBorder:
                return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
	        default: 
                R_CORE_ASSERT(false, "");
	        }
        }
    };
}

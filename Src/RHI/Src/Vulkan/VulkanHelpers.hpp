#pragma once

#include <RHI/Assert.hpp>
#include <RHI/BufferDescriptor.hpp>
#include <RHI/SamplerDescriptor.hpp>
#include <RHI/RenderPassDescriptor.hpp>
#include <RHI/PipelineDescriptor.hpp>

#pragma warning(push)
#pragma warning(disable : 4189)
#include <VulkanMemoryAllocator/vk_mem_alloc.h>
#pragma warning(pop)

#include <vulkan/vulkan.h>


#define HELPER_DEFAULT_RETURN(TypeName) RHI_ASSERT(false); \
                                        return static_cast<TypeName>(-1)

namespace rhi::vulkan::helpers
{

inline VkBufferUsageFlags BufferUsage(BufferType type)
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
        HELPER_DEFAULT_RETURN(VkBufferUsageFlags);
    }
}

inline VmaMemoryUsage MemoryUsage(MemoryType type)
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
        HELPER_DEFAULT_RETURN(VmaMemoryUsage);
    }
}

inline VkFormat Format(Format format)
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
    case Format::RGBA8_UNORM:
        return VK_FORMAT_R8G8B8A8_UNORM;
    default:
        HELPER_DEFAULT_RETURN(VkFormat);
    }
}

inline VkSamplerAddressMode AddressMode(AddressMode mode)
{
    switch (mode)
    {
    case AddressMode::REPEAT:
        return VK_SAMPLER_ADDRESS_MODE_REPEAT;
    case AddressMode::CLAMP_TO_EDGE:
        return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    case AddressMode::CLAMP_TO_BORDER:
        return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
    default:
        HELPER_DEFAULT_RETURN(VkSamplerAddressMode);
    }
}

inline VkAttachmentLoadOp LoadOperation(AttachmentLoadOperation loadOp)
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
        HELPER_DEFAULT_RETURN(VkAttachmentLoadOp);
    }
}

inline VkAttachmentStoreOp StoreOperation(AttachmentStoreOperation storeOp)
{
    switch (storeOp)
    {
    case AttachmentStoreOperation::UNDEFINED:
        return VK_ATTACHMENT_STORE_OP_DONT_CARE;
    case AttachmentStoreOperation::STORE:
        return VK_ATTACHMENT_STORE_OP_STORE;
    default:
        HELPER_DEFAULT_RETURN(VkAttachmentStoreOp);
    }
}

inline VkExtent2D Extent(const glm::ivec2& extent)
{
    return { static_cast<uint32_t>(extent.x), static_cast<uint32_t>(extent.y) };
}

inline VkRect2D Rect(const glm::ivec2& extent)
{
    VkRect2D rect;
    rect.extent = Extent(extent);
    rect.offset = { 0, 0 };
    return rect;
}

inline VkCullModeFlags CullMode(CullMode mode)
{
    switch (mode)
    {
    case CullMode::NONE:
        return VK_CULL_MODE_NONE;
    case CullMode::FRONT:
        return VK_CULL_MODE_FRONT_BIT;
    case CullMode::BACK:
        return VK_CULL_MODE_BACK_BIT;
    default:
        HELPER_DEFAULT_RETURN(VkCullModeFlags);
    }
}

inline VkShaderStageFlagBits ShaderStage(ShaderStage stage)
{
    switch (stage)
    {
    case ShaderStage::VERTEX:
        return VK_SHADER_STAGE_VERTEX_BIT;
    case ShaderStage::FRAGMENT:
        return VK_SHADER_STAGE_FRAGMENT_BIT;
    case ShaderStage::COMPUTE:
        return VK_SHADER_STAGE_COMPUTE_BIT;
    default:
        HELPER_DEFAULT_RETURN(VkShaderStageFlagBits);
    }
}

inline VkCompareOp CompareOp(CompareOp op)
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
        HELPER_DEFAULT_RETURN(VkCompareOp);
    }
}

} // namespace rhi::vulkan::helpers
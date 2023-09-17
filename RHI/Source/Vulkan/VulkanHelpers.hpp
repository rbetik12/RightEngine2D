#pragma once

#include <RHI/Assert.hpp>
#include <RHI/BufferDescriptor.hpp>
#include <vulkan/vulkan.h>

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
            RHI_ASSERT(false);
            return static_cast<VkBufferUsageFlags>(-1);
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
            RHI_ASSERT(false);
            return static_cast<VmaMemoryUsage>(-1);
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
        default:
            RHI_ASSERT(false);
            return static_cast<VkFormat>(-1);
        }
    }

} // namespace rhi::vulkan::helpers
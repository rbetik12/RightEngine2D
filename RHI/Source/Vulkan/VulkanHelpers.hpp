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
}
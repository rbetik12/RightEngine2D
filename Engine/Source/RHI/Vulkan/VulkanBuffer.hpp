#pragma once

#include "Buffer.hpp"
#include <VulkanMemoryAllocator/vk_mem_alloc.h>
#include <vulkan/vulkan.h>

namespace RightEngine
{
    class VulkanBuffer : public Buffer
    {
    public:
        VulkanBuffer(std::shared_ptr<Device> device, const BufferDescriptor& bufferDescriptor, const void* data);

        virtual ~VulkanBuffer() override;

        virtual void* Map() const override;
        virtual void UnMap() const override;

        VkBuffer GetBuffer() const
        { return buffer; }

    private:
        VkBuffer buffer{ VK_NULL_HANDLE };
        VmaAllocation allocation;
        // Is used only for constant buffer
        mutable uint8_t* bufferData{ nullptr };
    };
}
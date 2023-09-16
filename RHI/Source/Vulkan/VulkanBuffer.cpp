#include "VulkanBuffer.hpp"
#include "VulkanHelpers.hpp"

namespace rhi::vulkan
{
	namespace 
	{
        constexpr const uint8_t C_MAX_CONSTANT_BUFFER_SIZE = 128;
	}

	VulkanBuffer::VulkanBuffer(const BufferDescriptor& desc, const void* data) : Buffer(desc)
	{
        if (desc.m_type == BufferType::CONSTANT)
        {
            RHI_ASSERT(desc.m_size <= C_MAX_CONSTANT_BUFFER_SIZE);
            m_bufferData = new uint8_t[desc.m_size];
            std::memset(m_bufferData, 0, m_descriptor.m_size);
        }
        else
        {
            VkBufferCreateInfo bufferInfo{};
            bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
            bufferInfo.size = desc.m_size;
            bufferInfo.usage = helpers::BufferUsage(desc.m_type);
            bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

            VmaAllocationCreateInfo vmaAllocInfo = {};
            vmaAllocInfo.usage = helpers::MemoryUsage(desc.m_memoryType);

            const auto result = vmaCreateBuffer(VulkanDevice::s_ctx.m_allocator, 
                &bufferInfo, 
                &vmaAllocInfo,
                &m_buffer,
                &m_allocation,
                nullptr);

            if (result != VK_SUCCESS)
            {
                log::error("[Vulkan] Failed to allocate buffer '{}' with the size of {}B", desc.m_name, desc.m_size);
                return;
            }
            log::debug("[Vulkan] Successfully allocated buffer '{}' with the size of {}B", desc.m_name, desc.m_size);
        }

        if (data)
        {
            void* bufferPtr = Map();
            memcpy(bufferPtr, data, desc.m_size);
            UnMap();
        }
	}

	VulkanBuffer::~VulkanBuffer()
	{
        if (m_descriptor.m_type == BufferType::CONSTANT)
        {
            delete[] m_bufferData;
            return;
        }
        vmaDestroyBuffer(VulkanDevice::s_ctx.m_allocator, m_buffer, m_allocation);
	}

	void* VulkanBuffer::Map() const
	{
        if (m_descriptor.m_type == BufferType::CONSTANT)
        {
            if (!m_bufferData)
            {
                m_bufferData = new uint8_t[m_descriptor.m_size];
                std::memset(m_bufferData, 0, m_descriptor.m_size);
            }
            return m_bufferData;
        }
        void* data;
        vmaMapMemory(VulkanDevice::s_ctx.m_allocator, m_allocation, &data);
        return data;
	}

	void VulkanBuffer::UnMap() const
	{
        if (m_descriptor.m_type == BufferType::CONSTANT)
        {
            return;
        }
        vmaUnmapMemory(VulkanDevice::s_ctx.m_allocator, m_allocation);
	}
}

#pragma once

#include <RHI/Config.hpp>
#include <RHI/Buffer.hpp>
#include "VulkanDevice.hpp"

namespace rhi::vulkan
{
	class RHI_API VulkanBuffer : public Buffer
	{
	public:
		VulkanBuffer(const BufferDescriptor& desc, const void* data);

		virtual ~VulkanBuffer();

		virtual void*	Map() const;
		virtual void	UnMap() const;

	private:
		VkBuffer			m_buffer = nullptr;
		VmaAllocation		m_allocation = nullptr;

		// Is used only for constant buffer
		mutable uint8_t*	m_bufferData = nullptr;
	};
}

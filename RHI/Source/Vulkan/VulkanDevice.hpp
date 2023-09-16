#pragma once

#include <RHI/Config.hpp>
#include <RHI/Device.hpp>
#include "VulkanContext.hpp"
#include <VulkanMemoryAllocator/vk_mem_alloc.h>

namespace rhi::vulkan
{

	class RHI_API VulkanDevice : public Device
	{
	public:
		// Global context needed for operations with memory (buffers, texture and other allocations)
		struct ContextHolder
		{
			VkDevice		m_device = nullptr;
			VmaAllocator	m_allocator = nullptr;
		};

		inline static ContextHolder s_ctx;

	public:
		VulkanDevice(const std::shared_ptr<VulkanContext>& context);

		virtual ~VulkanDevice() override;

		virtual std::shared_ptr<ShaderCompiler> CreateShaderCompiler(const ShaderCompiler::Options& options = {}) override;
		virtual std::shared_ptr<Buffer>			CreateBuffer(const BufferDescriptor& desc, const void* data) override;

		VkPhysicalDevice						PhysicalDevice() const { return m_physicalDevice; }

	private:
		VkPhysicalDevice	m_physicalDevice = nullptr;
		VkQueue				m_graphicsQueue = nullptr;
		VkQueue				m_presentQueue = nullptr;

		// Initializer methods
		void				PickPhysicalDevice(const std::shared_ptr<VulkanContext>& context);
		void				CreateLogicalDevice(const std::shared_ptr<VulkanContext>& context);
		void				SetupDeviceQueues();
		void				FillProperties();
		void				SetupAllocator(const std::shared_ptr<VulkanContext>& context);
	};

}

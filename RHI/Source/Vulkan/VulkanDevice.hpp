#pragma once

#include <RHI/Config.hpp>
#include <Rhi/Device.hpp>
#include <VulkanMemoryAllocator/vk_mem_alloc.h>

#include "VulkanContext.hpp"

namespace rhi::vulkan
{

	class RHI_API VulkanDevice : public Device
	{
	public:
		VulkanDevice(const std::shared_ptr<VulkanContext>& context);

		virtual ~VulkanDevice() override;

		virtual std::shared_ptr<ShaderCompiler> CreateShaderCompiler(const ShaderCompiler::Options& options = {}) override;

	private:
		VkPhysicalDevice				m_physicalDevice =  VK_NULL_HANDLE;
		VkDevice						m_device = VK_NULL_HANDLE;
		VkQueue							m_graphicsQueue;
		VkQueue							m_presentQueue;
		VmaAllocator					m_allocator;

		// Initializer methods
		void				PickPhysicalDevice(const std::shared_ptr<VulkanContext>& context);
		void				CreateLogicalDevice(const std::shared_ptr<VulkanContext>& context);
		void				SetupDeviceQueues();
		void				SetupAllocator();
		void				FillProperties();
	};

}

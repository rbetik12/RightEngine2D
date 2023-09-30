#pragma once

#include <RHI/Config.hpp>
#include <RHI/Device.hpp>
#include "VulkanContext.hpp"
#include "CommandBuffer.hpp"
#include "Fence.hpp"
#include <vulkan/vulkan.hpp>
#include <VulkanMemoryAllocator/vk_mem_alloc.h>

namespace rhi::vulkan
{

	class RHI_API VulkanDevice : public Device
	{
	public:
		// Physical device properties cache
		struct Properties
		{
			size_t	m_minUniformBufferOffsetAlignment = 0;
			float	m_maxSamplerAnisotropy = 0;
		};

		// Global context needed for operations with memory (buffers, texture and other allocations)
		struct ContextHolder
		{
			VkDevice			m_device = nullptr;
			VmaAllocator		m_allocator = nullptr;
			VkPhysicalDevice	m_physicalDevice = nullptr;
			Properties			m_properties;
			VulkanDevice*		m_instance = nullptr;
		};

		inline static ContextHolder s_ctx;

	public:
		VulkanDevice(const std::shared_ptr<VulkanContext>& context);

		virtual ~VulkanDevice() override;

		virtual std::shared_ptr<ShaderCompiler> CreateShaderCompiler(const ShaderCompiler::Options& options = {}) override;
		virtual std::shared_ptr<Buffer>			CreateBuffer(const BufferDescriptor& desc, const void* data) override;
		virtual std::shared_ptr<Shader>			CreateShader(const ShaderDescriptor& desc) override;
		virtual std::shared_ptr<Sampler>		CreateSampler(const SamplerDescriptor& desc) override;

		VkPhysicalDevice						PhysicalDevice() const { return s_ctx.m_physicalDevice; }
		VkCommandPool							CommandPool() const { return m_commandPool; }

		Fence									Execute(CommandBuffer buffer);

	private:
		VkQueue				m_graphicsQueue = nullptr;
		VkQueue				m_presentQueue = nullptr;
		VkCommandPool		m_commandPool = nullptr;

		// Initializer methods
		void				PickPhysicalDevice(const std::shared_ptr<VulkanContext>& context);
		void				CreateLogicalDevice(const std::shared_ptr<VulkanContext>& context);
		void				SetupDeviceQueues(const std::shared_ptr<VulkanContext>& context);
		void				FillProperties();
		void				SetupAllocator(const std::shared_ptr<VulkanContext>& context);
		void				SetupCommandPool(const std::shared_ptr<VulkanContext>& context);
	};

}

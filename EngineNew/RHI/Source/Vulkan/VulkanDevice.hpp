#pragma once

#include <optional>
#include <RHI/Config.hpp>
#include <RHI/Device.hpp>
#include "VulkanContext.hpp"
#include "CommandBuffer.hpp"
#include "Fence.hpp"
#include "Swapchain.hpp"
#include <VulkanMemoryAllocator/vk_mem_alloc.h>

#include "Semaphore.hpp"

namespace rhi::vulkan
{

struct SwapchainSupportDetails
{
	VkSurfaceCapabilitiesKHR			m_capabilities;
	eastl::vector<VkSurfaceFormatKHR>	m_formats;
	eastl::vector<VkPresentModeKHR>		m_presentModes;
};

struct QueueFamilyIndices
{
	std::optional<uint32_t> graphicsFamily;
	std::optional<uint32_t> presentFamily;

	bool IsComplete() const
	{
		return graphicsFamily.has_value() && presentFamily.has_value();
	}
};

class RHI_API VulkanDevice : public Device
{
public:
	// Device properties cache
	struct Properties
	{
		uint32_t	m_framesInFlight = 2;
		size_t		m_minUniformBufferOffsetAlignment = 0;
		float		m_maxSamplerAnisotropy = 0;
	};

	// Global context needed for operations with memory (buffers, texture and other allocations)
	struct ContextHolder
	{
		VkDevice			m_device = nullptr;
		VmaAllocator		m_allocator = nullptr;
		VkPhysicalDevice	m_physicalDevice = nullptr;
		VkSurfaceKHR		m_surface = nullptr;
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
	virtual std::shared_ptr<Texture>		CreateTexture(const TextureDescriptor& desc, const void* data = {}) override;
	virtual std::shared_ptr<RenderPass>		CreateRenderPass(const RenderPassDescriptor& desc) override;
	virtual std::shared_ptr<Pipeline>		CreatePipeline(const PipelineDescriptor& desc) override;

	virtual void							BeginFrame() override;
	virtual void							EndFrame() override;
	virtual void							BeginPipeline(const std::shared_ptr<Pipeline>& pipeline) override;
	virtual void							EndPipeline(const std::shared_ptr<Pipeline>& pipeline) override;

	VkPhysicalDevice						PhysicalDevice() const { return s_ctx.m_physicalDevice; }
	VkCommandPool							CommandPool() const { return m_commandPool; }
	const SwapchainSupportDetails&			GetSwapchainSupportDetails() const { return m_swapchainDetails; }
	QueueFamilyIndices						FindQueueFamilies() const;

	Fence									Execute(CommandBuffer buffer);

private:
	VkQueue						m_graphicsQueue = nullptr;
	VkQueue						m_presentQueue = nullptr;
	VkCommandPool				m_commandPool = nullptr;
	SwapchainSupportDetails		m_swapchainDetails;
	std::unique_ptr<Swapchain>	m_swapchain;
	uint32_t					m_frameIndex = 0;
	uint32_t					m_currentCmdBufferIndex = 0;

	// TODO: Rename to a better name
	struct CommandBufferSync
	{
		CommandBuffer	m_buffer;
		Fence			m_fence;
		Semaphore       m_imageAvailableSemaphore;
		Semaphore       m_renderFinishedSemaphore;
	};
	eastl::vector<CommandBufferSync> m_cmdBuffers;

	// Initializer methods
	void PickPhysicalDevice(const std::shared_ptr<VulkanContext>& context);
	void CreateLogicalDevice(const std::shared_ptr<VulkanContext>& context);
	void SetupDeviceQueues(const std::shared_ptr<VulkanContext>& context);
	void FillProperties();
	void SetupAllocator(const std::shared_ptr<VulkanContext>& context);
	void SetupCommandPool(const std::shared_ptr<VulkanContext>& context);
	void FillSwapchainSupportDetails(const std::shared_ptr<VulkanContext>& context);
};

}

#pragma once

#include <optional>
#include <RHI/Config.hpp>
#include <RHI/Device.hpp>
#include "VulkanContext.hpp"
#include "CommandBuffer.hpp"
#include "Fence.hpp"
#include "Swapchain.hpp"
#include <VulkanMemoryAllocator/vk_mem_alloc.h>

namespace rhi::vulkan
{

class VulkanTexture;

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
	virtual void							Present() override;
	virtual void							BeginPipeline(const std::shared_ptr<Pipeline>& pipeline) override;
	virtual void							EndPipeline(const std::shared_ptr<Pipeline>& pipeline) override;
	virtual void							Draw(const std::shared_ptr<Buffer>& buffer, uint32_t vertexCount, uint32_t instanceCount) override;

	virtual void							OnResize(uint32_t x, uint32_t y) override;

	VkPhysicalDevice						PhysicalDevice() const { return s_ctx.m_physicalDevice; }
	VkCommandPool							CommandPool() const { return m_commandPool; }
	const SwapchainSupportDetails&			GetSwapchainSupportDetails() const { return m_swapchainDetails; }
	QueueFamilyIndices						FindQueueFamilies() const;

	std::shared_ptr<Fence>					Execute(CommandBuffer buffer);

private:
	VkQueue						m_graphicsQueue = nullptr;
	VkQueue						m_presentQueue = nullptr;
	VkCommandPool				m_commandPool = nullptr;
	SwapchainSupportDetails		m_swapchainDetails;
	std::unique_ptr<Swapchain>	m_swapchain;
	uint32_t					m_frameIndex = 0;
	uint32_t					m_swapchainImageIndex = 0;
	uint32_t					m_currentCmdBufferIndex = 0;
	glm::ivec2					m_presentExtent = {0, 0};
	bool						m_isSwapchainDirty = false;

	eastl::vector<VkCommandBuffer>					m_cmdBuffers;
	eastl::vector<VkFence>							m_fences;
	eastl::vector<VkSemaphore>						m_presentSemaphores;
	eastl::vector<VkSemaphore>						m_renderSemaphores;
	eastl::vector<std::shared_ptr<VulkanTexture>>	m_texturesToReset;

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

#pragma once

#include <optional>
#include <RHI/Config.hpp>
#include <RHI/Device.hpp>
#include "VulkanContext.hpp"
#include "CommandBuffer.hpp"
#include "Fence.hpp"
#include "Swapchain.hpp"

#pragma warning(push)
#pragma warning(disable : 4189)
#include <VulkanMemoryAllocator/vk_mem_alloc.h>
#pragma warning(pop)

namespace rhi::vulkan
{

class VulkanTexture;

struct SwapchainSupportDetails
{
    VkSurfaceCapabilitiesKHR            m_capabilities = {};
    eastl::vector<VkSurfaceFormatKHR>   m_formats;
    eastl::vector<VkPresentModeKHR>     m_presentModes;
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
    // Global context needed for operations with memory (buffers, texture and other allocations)
    struct ContextHolder
    {
        VkDevice            m_device = nullptr;
        VmaAllocator        m_allocator = nullptr;
        VkPhysicalDevice    m_physicalDevice = nullptr;
        VulkanDevice*       m_instance = nullptr;
    };

    inline static ContextHolder s_ctx;

public:
    VulkanDevice(const std::shared_ptr<VulkanContext>& context);

    virtual ~VulkanDevice() override;

    virtual std::shared_ptr<ShaderCompiler>     CreateShaderCompiler(const ShaderCompiler::Options& options = {}) override;
    virtual std::shared_ptr<Buffer>             CreateBuffer(const BufferDescriptor& desc, const void* data) override;
    virtual std::shared_ptr<Shader>             CreateShader(const ShaderDescriptor& desc) override;
    virtual std::shared_ptr<Sampler>            CreateSampler(const SamplerDescriptor& desc) override;
    virtual std::shared_ptr<Texture>            CreateTexture(const TextureDescriptor& desc, const std::shared_ptr<Sampler>& sampler, const void* data = {}) override;
    virtual std::shared_ptr<RenderPass>         CreateRenderPass(const RenderPassDescriptor& desc) override;
    virtual std::shared_ptr<Pipeline>           CreatePipeline(const PipelineDescriptor& desc) override;
    virtual std::shared_ptr<GPUMaterial>        CreateGPUMaterial(const std::shared_ptr<Shader>& shader) override;

    virtual void                            BeginFrame() override;
    virtual void                            EndFrame() override;
    virtual void                            BeginComputePipeline(const std::shared_ptr<Pipeline>& pipeline) override;
    virtual void                            EndComputePipeline(const std::shared_ptr<Pipeline>& pipeline) override;
    virtual std::shared_ptr<ComputeState>   BeginComputePipelineImmediate(const std::shared_ptr<Pipeline>& pipeline) override;
    virtual void                            EndComputePipeline(const std::shared_ptr<Pipeline>& pipeline, const std::shared_ptr<ComputeState>& state) override;
    virtual void                            PushConstantComputeImmediate(const void* data, uint32_t size, const std::shared_ptr<Pipeline>& pipeline, const std::shared_ptr<ComputeState>& state) override;
    virtual void                            Present() override;
    virtual void                            BeginPipeline(const std::shared_ptr<Pipeline>& pipeline) override;
    virtual void                            EndPipeline(const std::shared_ptr<Pipeline>& pipeline) override;
    virtual void                            Draw(const std::shared_ptr<Buffer>& buffer, uint32_t vertexCount, uint32_t instanceCount) override;
    virtual void                            Draw(const std::shared_ptr<Buffer>& vb, const std::shared_ptr<Buffer>& ib, uint32_t indexCount, uint32_t instanceCount) override;
    virtual void                            Dispatch(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ) override;
    virtual void                            Dispatch(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ, const std::shared_ptr<ComputeState>& state) override;
    virtual void                            BindGPUMaterial(const std::shared_ptr<GPUMaterial>& material, const std::shared_ptr<Pipeline>& pipeline) override;
    virtual void                            BindGPUMaterial(const std::shared_ptr<GPUMaterial>& material, const std::shared_ptr<Pipeline>& pipeline, const std::shared_ptr<ComputeState>& state) override;
    virtual void                            PushConstant(const void* data, uint32_t size, const std::shared_ptr<Pipeline>& pipeline) override;

    virtual void                            OnResize(uint32_t x, uint32_t y) override;

    virtual void                            WaitForIdle() override;

    VkPhysicalDevice                        PhysicalDevice() const { return s_ctx.m_physicalDevice; }
    VkCommandPool                           CommandPool() const { return m_commandPool; }
    const SwapchainSupportDetails&          GetSwapchainSupportDetails() const { return m_swapchainDetails; }
    QueueFamilyIndices                      FindQueueFamilies() const;
    const std::shared_ptr<VulkanContext>&   Context() const { return m_context; }
    VkQueue                                 GraphicsQueue() const { return m_graphicsQueue; }
    VkCommandBuffer                         CurrentCmdBuffer() const { return m_cmdBuffers[m_currentCmdBufferIndex]; }

    std::shared_ptr<Fence>                  Execute(CommandBuffer buffer);

private:
    VkQueue                         m_graphicsQueue = nullptr;
    VkQueue                         m_presentQueue = nullptr;
    VkCommandPool                   m_commandPool = nullptr;
    SwapchainSupportDetails         m_swapchainDetails;
    std::unique_ptr<Swapchain>      m_swapchain;
    uint32_t                        m_frameIndex = 0;
    uint32_t                        m_swapchainImageIndex = 0;
    uint32_t                        m_currentCmdBufferIndex = 0;
    glm::ivec2                      m_presentExtent = {0, 0};
    bool                            m_isSwapchainDirty = false;
    std::shared_ptr<VulkanContext>  m_context;

    eastl::vector<VkCommandBuffer>                  m_cmdBuffers;
    // TODO: It is quick and easy implementation in future with must integrate compute cmd buffers to general rendering pipeline
    eastl::vector<VkCommandBuffer>                  m_computeCmdBuffers;
    eastl::vector<VkFence>                          m_fences;
    eastl::vector<VkFence>                          m_computeFences;
    eastl::vector<VkSemaphore>                      m_presentSemaphores;
    eastl::vector<VkSemaphore>                      m_renderSemaphores;
    eastl::vector<VkSemaphore>                      m_computeSemaphores;
    eastl::vector<std::shared_ptr<VulkanTexture>>   m_texturesToReset;
    eastl::vector<std::shared_ptr<VulkanTexture>>   m_computeTexturesToReset;

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

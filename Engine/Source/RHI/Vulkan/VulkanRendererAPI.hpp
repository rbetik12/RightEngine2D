#pragma once

#include "RendererAPI.hpp"
#include "VulkanRenderingContext.hpp"
#include "VulkanSurface.hpp"
#include "VulkanSwapchain.hpp"
#include "VulkanGraphicsPipeline.hpp"
#include "VulkanCommandBuffer.hpp"

namespace RightEngine
{
    class VulkanRendererAPI : public RendererAPI
    {
    public:
        virtual void Init() override;

        virtual void BeginFrame(const std::shared_ptr<CommandBuffer>& cmd,
                                const std::shared_ptr<GraphicsPipeline>& pipeline) override;
        virtual void EndFrame(const std::shared_ptr<CommandBuffer>& cmd,
                              const std::shared_ptr<GraphicsPipeline>& pipeline) override;

        virtual void Configure(const RendererSettings& settings) override;

        virtual void SetClearColor(const glm::vec4& color) override;

        virtual void Clear(uint32_t clearBits) override;

        virtual void SetViewport(const Viewport& viewport) override;
        virtual Viewport GetViewport() override;

        virtual void Draw(const std::shared_ptr<CommandBuffer>& cmd,
                          const std::shared_ptr<Buffer>& vertexBuffer,
                          const std::shared_ptr<Buffer>& indexBuffer,
                          uint32_t vertexCount,
                          uint32_t instanceCount) override;
        virtual void Draw(const std::shared_ptr<CommandBuffer>& cmd,
                          const std::shared_ptr<Buffer>& buffer,
                          uint32_t indexCount,
                          uint32_t instanceCount) override;

        virtual void EncodeState(const std::shared_ptr<CommandBuffer>& cmd,
                                 const std::shared_ptr<GraphicsPipeline>& pipeline,
                                 const std::shared_ptr<RendererState>& state) override;

        virtual std::shared_ptr<RendererState> CreateRendererState() override;

        virtual const std::shared_ptr<RenderingContext>& GetContext() const override;

        virtual ~VulkanRendererAPI() override;

        void SetImageLayout(const VulkanCommandBuffer* cmd,
                            VkImage image,
                            VkImageLayout oldLayout,
                            VkImageLayout newLayout,
                            int layerCount,
                            int mipmaps) const;

    private:
        std::shared_ptr<VulkanRenderingContext> context;
        std::shared_ptr<VulkanSurface> surface;
        std::shared_ptr<VulkanSwapchain> swapchain;
        std::vector<VkFramebuffer> swapchainFramebuffers;
        std::vector<VkSemaphore> imageAvailableSemaphores;
        std::vector<VkSemaphore> renderFinishedSemaphores;
        std::vector<VkFence> inFlightFences;
        VkImageCopy imageCopy;
        mutable std::unordered_map<VkImage, VkImageLayout> imageLayouts;

        void CreateSyncObjects();
        void CreateSwapchain();
        void DestroySwapchain();
    };
}
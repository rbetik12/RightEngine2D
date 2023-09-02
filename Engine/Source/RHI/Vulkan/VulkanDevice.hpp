#pragma once

#include "Device.hpp"
#include "VulkanRenderingContext.hpp"
#include "VulkanSurface.hpp"
#include <VulkanMemoryAllocator/vk_mem_alloc.h>
#include <vulkan/vulkan.h>
#include <optional>

namespace RightEngine
{
    enum class QueueType
            {
        PRESENT,
        GRAPHICS,
            };
    struct QueueFamilyIndices
    {
        std::optional<uint32_t> graphicsFamily;
        std::optional<uint32_t> presentFamily;

        bool IsComplete() const
        {
            return graphicsFamily.has_value()
                   && presentFamily.has_value();
        }
    };

    struct SwapchainSupportDetails
    {
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> presentModes;
    };

    class VulkanDevice : public Device
    {
    public:
        VulkanDevice(const std::shared_ptr<RenderingContext>& ctx,
                     const std::shared_ptr<Surface>& surface);
        virtual ~VulkanDevice() override;

        virtual std::shared_ptr<Buffer> CreateBuffer( const BufferDescriptor& bufferDescriptor, const void* data) override;
        virtual std::shared_ptr<CommandBuffer> CreateCommandBuffer(const CommandBufferDescriptor& descriptor) override;
        virtual std::shared_ptr<Shader> CreateShader(const ShaderProgramDescriptor& shaderProgramDescriptor) override;
        virtual std::shared_ptr<GraphicsPipeline> CreateGraphicsPipeline(const GraphicsPipelineDescriptor& descriptor,
                                                                         const RenderPassDescriptor& renderPassDescriptor) override;
        virtual std::shared_ptr<Texture> CreateTexture(const TextureDescriptor& descriptor,
                                                       const std::vector<uint8_t>& data) override;

        virtual std::shared_ptr<Sampler> CreateSampler(const SamplerDescriptor& descriptor) override;

        VkPhysicalDevice GetPhysicalDevice() const
        { return physicalDevice; }

        VkDevice GetDevice() const
        { return device; }

        VkQueue GetQueue(QueueType type) const
        {
            switch (type)
            {
                case QueueType::PRESENT:
                    return presentQueue;
                case QueueType::GRAPHICS:
                    return graphicsQueue;
            }
        }

        SwapchainSupportDetails GetSwapchainSupportDetails() const;
        QueueFamilyIndices FindQueueFamilies() const;
        VmaAllocator GetAllocator() const
        { return allocator; }

    private:
        VkPhysicalDevice physicalDevice{ VK_NULL_HANDLE };
        VkDevice device{ VK_NULL_HANDLE };
        VkQueue graphicsQueue;
        VkQueue presentQueue;
        VmaAllocator allocator;

        void Init(const std::shared_ptr<VulkanRenderingContext>& context);
        void PickPhysicalDevice(const std::shared_ptr<VulkanRenderingContext>& context);
        void CreateLogicalDevice(const std::shared_ptr<VulkanRenderingContext>& context);
        void SetupDeviceQueues(const std::shared_ptr<VulkanRenderingContext>& context);
        void SetupAllocator(const std::shared_ptr<VulkanRenderingContext>& context);
    };
}
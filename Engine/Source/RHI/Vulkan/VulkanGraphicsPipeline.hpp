#pragma once

#include "VulkanSwapchain.hpp"
#include "GraphicsPipeline.hpp"
#include <vulkan/vulkan.h>

namespace RightEngine
{
    class VulkanGraphicsPipeline : public GraphicsPipeline
    {
    public:
        VulkanGraphicsPipeline(const GraphicsPipelineDescriptor& descriptor,
                               const RenderPassDescriptor& renderPassDescriptor);

        ~VulkanGraphicsPipeline();
        
        virtual void Resize(int x, int y) override;

        VkRenderPass GetRenderPass() const
        { return renderPass; }

        VkPipeline GetPipeline() const
        { return graphicsPipeline; }

        VkPipelineLayout GetPipelineLayout() const
        { return pipelineLayout; }

        VkDescriptorSetLayout GetDescriptorSetLayout() const
        { return descriptorSetLayout; }

        VkFramebuffer GetFramebuffer() const
        { return framebuffer; }

        const std::vector<VkClearValue>& ClearValue() const
        { return m_clearValues; }

    private:
        VkPipeline graphicsPipeline{ VK_NULL_HANDLE };
        VkPipelineLayout pipelineLayout{ VK_NULL_HANDLE };
        VkRenderPass renderPass{ VK_NULL_HANDLE };
        VkDescriptorSetLayout descriptorSetLayout{ VK_NULL_HANDLE };
        std::vector<VkPushConstantRange> pushConstants;
        VkFramebuffer framebuffer{ VK_NULL_HANDLE };
        std::vector<VkClearValue> m_clearValues;

        void Init(const GraphicsPipelineDescriptor& descriptorconst,
                  const RenderPassDescriptor& renderPassDescriptor);
        void CreateRenderPass(const RenderPassDescriptor& renderPassDescriptor);
        void CreateDescriptorSets();
        void CreateFramebuffer();
    };
}

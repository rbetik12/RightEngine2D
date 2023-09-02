#pragma once

#include "RendererState.hpp"
#include <vulkan/vulkan.h>

namespace RightEngine
{
    class VulkanRendererState : public RendererState
    {
    public:
        virtual void OnUpdate(const std::shared_ptr<GraphicsPipeline>& pipeline) override;

        const VkDescriptorSet GetDescriptorSet() const
        { return descriptorSet; }

        virtual ~VulkanRendererState() override;

    private:
        VkDescriptorSet descriptorSet{ VK_NULL_HANDLE };
        VkDescriptorPool descriptorPool{ VK_NULL_HANDLE };
    };
}
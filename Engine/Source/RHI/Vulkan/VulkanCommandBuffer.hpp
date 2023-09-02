#pragma once

#include "CommandBuffer.hpp"
#include <vulkan/vulkan.h>

#define MAX_COMMAND_BUFFER_SIZE 16384
#define VK_CMD(x) static_cast<VulkanCommandBuffer*>(x)

namespace RightEngine
{
    class VulkanCommandBuffer : public CommandBuffer
    {
    public:
        VulkanCommandBuffer(const std::shared_ptr<Device>& device, const CommandBufferDescriptor& descriptor);

        virtual ~VulkanCommandBuffer();
        virtual void Enqueue(std::function<void(CommandBuffer*)>&& command) override;
        virtual void Execute() override;

        VkCommandBuffer GetBuffer() const
        { return commandBuffer; }

    private:
        VkCommandPool commandPool;
        VkCommandBuffer commandBuffer;

        std::function<void(CommandBuffer*)> commands[MAX_COMMAND_BUFFER_SIZE];
        uint32_t commandsAmount{ 0 };
    };
}
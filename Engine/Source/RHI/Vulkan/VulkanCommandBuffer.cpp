#include "VulkanCommandBuffer.hpp"
#include "VulkanDevice.hpp"

using namespace RightEngine;

VulkanCommandBuffer::VulkanCommandBuffer(const std::shared_ptr<Device>& device, const CommandBufferDescriptor& descriptor) : CommandBuffer(device, descriptor)
{
    const auto vkDevice = std::static_pointer_cast<VulkanDevice>(device);
    QueueFamilyIndices queueFamilyIndices = vkDevice->FindQueueFamilies();

    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

    if (vkCreateCommandPool(vkDevice->GetDevice(), &poolInfo, nullptr, &commandPool) != VK_SUCCESS)
    {
        R_CORE_ASSERT(false,"failed to create command pool!");
    }
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = 1;

    if (vkAllocateCommandBuffers(vkDevice->GetDevice(), &allocInfo, &commandBuffer) != VK_SUCCESS)
    {
        R_CORE_ASSERT(false, "failed to allocate command buffers!");
    }
}

void VulkanCommandBuffer::Enqueue(std::function<void(CommandBuffer*)>&& command)
{
    R_CORE_ASSERT(commandsAmount < MAX_COMMAND_BUFFER_SIZE, "");
    commands[commandsAmount] = command;
    commandsAmount += 1;
}

void VulkanCommandBuffer::Execute()
{
    for (uint32_t i = 0; i < commandsAmount; i++)
    {
        commands[i](this);
    }
    commandsAmount = 0;
}

VulkanCommandBuffer::~VulkanCommandBuffer()
{
    vkDestroyCommandPool(VK_DEVICE()->GetDevice(), commandPool, nullptr);
}

#include "CommandBuffer.hpp"
#include "VulkanDevice.hpp"

namespace rhi::vulkan
{

CommandBuffer::CommandBuffer()
{
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = VulkanDevice::s_ctx.m_instance->CommandPool();
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = 1;

    RHI_ASSERT(vkAllocateCommandBuffers(VulkanDevice::s_ctx.m_device, &allocInfo, &m_handle) == VK_SUCCESS);
}

CommandBuffer::~CommandBuffer()
{
}

void CommandBuffer::Begin(bool oneTimeUsage)
{
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    if (oneTimeUsage)
    {
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    }

    const auto result = vkBeginCommandBuffer(m_handle, &beginInfo);
    RHI_ASSERT(result == VK_SUCCESS);
}

void CommandBuffer::End()
{
    const auto result = vkEndCommandBuffer(m_handle);
    RHI_ASSERT(result == VK_SUCCESS);
}

void CommandBuffer::Reset()
{
    const auto result = vkResetCommandBuffer(m_handle, 0);
    RHI_ASSERT(result == VK_SUCCESS);
}

} // namespace rhi::vulkan

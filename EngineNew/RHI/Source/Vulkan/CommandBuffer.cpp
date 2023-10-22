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

	vkBeginCommandBuffer(m_handle, &beginInfo);
}

void CommandBuffer::End()
{
	vkEndCommandBuffer(m_handle);
}

} // namespace rhi::vulkan

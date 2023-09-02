#include "VulkanUtils.hpp"
#include "VulkanDevice.hpp"

using namespace RightEngine;

std::mutex VulkanUtils::s_mutex;

uint32_t VulkanUtils::FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
	VkPhysicalDeviceMemoryProperties memProperties;
	vkGetPhysicalDeviceMemoryProperties(VK_DEVICE()->GetPhysicalDevice(), &memProperties);

	for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
	{
		if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
		{
			return i;
		}
	}

	R_CORE_ASSERT(false, "failed to find suitable memory type!");
	return -1;
}

VkDeviceMemory VulkanUtils::AllocateMemory(VkMemoryRequirements memRequirements, VkMemoryPropertyFlags flags)
{
	VkMemoryAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits, flags);

	VkDeviceMemory memory;

	if (vkAllocateMemory(VK_DEVICE()->GetDevice(), &allocInfo, nullptr, &memory) != VK_SUCCESS)
	{
		R_CORE_ASSERT(false, "failed to allocate memory!");
		return nullptr;
	}

	return memory;
}

void VulkanUtils::BeginCommandBuffer(const std::shared_ptr<VulkanCommandBuffer>& cmd, bool oneTimeUsage)
{
	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	if (oneTimeUsage)
	{
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	}

	vkBeginCommandBuffer(cmd->GetBuffer(), &beginInfo);
}

void VulkanUtils::EndCommandBuffer(const std::shared_ptr<VulkanDevice>& device,
                                   const std::shared_ptr<VulkanCommandBuffer>& cmd)
{
	cmd->Execute();
	VkCommandBuffer commandBuffer = cmd->GetBuffer();

	vkEndCommandBuffer(commandBuffer);

	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;

	std::lock_guard lock(s_mutex);
	vkQueueSubmit(device->GetQueue(QueueType::GRAPHICS), 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(device->GetQueue(QueueType::GRAPHICS));
}

void VulkanUtils::EndCommandBuffer(const std::shared_ptr<VulkanDevice>& device,
                                   const std::shared_ptr<VulkanCommandBuffer>& cmd,
                                   const VkSubmitInfo& info,
                                   VkFence fence)
{
	cmd->Execute();
	VkCommandBuffer commandBuffer = cmd->GetBuffer();

	vkEndCommandBuffer(commandBuffer);

	std::lock_guard lock(s_mutex);
	vkQueueSubmit(device->GetQueue(QueueType::GRAPHICS), 1, &info, fence);
}

void VulkanUtils::CopyBuffer(const std::shared_ptr<VulkanCommandBuffer>& cmd, VkBuffer dst, VkBuffer src, size_t size)
{
	BeginCommandBuffer(cmd, true);

	VkBufferCopy bufferCopy;
	bufferCopy.size = size;

	cmd->Enqueue([=](auto buffer)
	{
		vkCmdCopyBuffer(VK_CMD(buffer)->GetBuffer(), src, dst, 1, &bufferCopy);
	});

	EndCommandBuffer(VK_DEVICE(), cmd);
}

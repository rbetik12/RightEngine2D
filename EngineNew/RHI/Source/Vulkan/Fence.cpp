#include "Fence.hpp"
#include "VulkanDevice.hpp"

namespace rhi::vulkan
{
	Fence::Fence()
	{
		VkFenceCreateInfo fenceInfo{};
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;

		RHI_ASSERT(vkCreateFence(VulkanDevice::s_ctx.m_device, &fenceInfo, nullptr, &m_handle) != VK_SUCCESS);
	}

	Fence::~Fence()
	{
		vkDestroyFence(VulkanDevice::s_ctx.m_device, m_handle, nullptr);
	}

	void Fence::Wait()
	{
		vkWaitForFences(VulkanDevice::s_ctx.m_device, 1, &m_handle, VK_TRUE, UINT64_MAX);
		vkResetFences(VulkanDevice::s_ctx.m_device, 1, &m_handle);
	}
}

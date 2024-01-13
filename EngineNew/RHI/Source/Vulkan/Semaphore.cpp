#include "Semaphore.hpp"
#include "VulkanDevice.hpp"

namespace rhi::vulkan
{

Semaphore::Semaphore()
{
	VkSemaphoreCreateInfo semaphoreInfo{};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	RHI_ASSERT(vkCreateSemaphore(VulkanDevice::s_ctx.m_device, &semaphoreInfo, nullptr, &m_handle) == VK_SUCCESS);
}

Semaphore::~Semaphore()
{
	vkDestroySemaphore(VulkanDevice::s_ctx.m_device, m_handle, nullptr);
}

}

#pragma once

#include <RHI/Config.hpp>
#include <vulkan/vulkan.h>

namespace rhi::vulkan
{

class RHI_API Semaphore
{
public:
	Semaphore();
	~Semaphore();

	VkSemaphore* Raw() { return &m_handle; }

private:
	VkSemaphore m_handle;
};

}
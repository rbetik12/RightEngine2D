#pragma once

#include <RHI/Config.hpp>
#include <vulkan/vulkan.h>

namespace rhi::vulkan
{
	class RHI_API Fence
	{
	public:
		Fence();
		~Fence();

		void Wait();

		VkFence Raw() const { return m_handle; }

	private:
		VkFence m_handle;
	};
}
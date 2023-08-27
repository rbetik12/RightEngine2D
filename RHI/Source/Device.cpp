#include <RHI/Device.hpp>
#include "Vulkan/VulkanDevice.hpp"

namespace rhi
{
	std::shared_ptr<Device> Device::Create()
	{
		static bool created = false;
		if (!created)
		{
			created = true;
			return std::make_shared<rhi::vulkan::VulkanDevice>();
		}
		RHI_ASSERT(false);
		return nullptr;
	}
}

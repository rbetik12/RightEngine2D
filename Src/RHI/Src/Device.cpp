#include <RHI/Device.hpp>
#include "Vulkan/VulkanDevice.hpp"
#include "Vulkan/VulkanContext.hpp"

namespace rhi
{
    std::shared_ptr<Device> Device::Create(const std::shared_ptr<IContext>& context)
    {
        static bool created = false;
        if (!created)
        {
            created = true;
            return std::make_shared<vulkan::VulkanDevice>(std::static_pointer_cast<vulkan::VulkanContext>(context));
        }
        RHI_ASSERT(false);
        return nullptr;
    }
}

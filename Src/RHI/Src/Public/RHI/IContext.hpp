#pragma once

#include <RHI/Config.hpp>
#include <Core/Type.hpp>
#include <vulkan/vulkan.h>
#include <memory>
#include <functional>

namespace rhi
{

class RHI_API IContext : public core::NonCopyable
{
public:
    virtual ~IContext() = default;

protected:
    IContext() = default;
};

namespace vulkan
{

// To be filled by user
struct VulkanInitContext
{
    using VulkanSurfaceFn = std::function<VkSurfaceKHR(VkInstance)>;

    VulkanSurfaceFn                m_surfaceConstructor;
    eastl::vector<const char*>    m_requiredExtensions;
};

std::shared_ptr<IContext> RHI_API CreateContext(VulkanInitContext&& ctx);

}

}
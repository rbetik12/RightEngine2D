#pragma once

#include <RHI/Config.hpp>
#include <RHI/Types.hpp>
#include <vulkan/vulkan.h>
#include <glm/glm.hpp>

namespace rhi::vulkan
{

struct SwapchainDescriptor
{
    Format format;
    VkPresentModeKHR presentMode;
    glm::ivec2 extent;
};

class RHI_API Swapchain
{
public:
    Swapchain(const SwapchainDescriptor& desc);
    ~Swapchain();

private:
};

}
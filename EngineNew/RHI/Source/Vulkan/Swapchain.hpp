#pragma once

#include <RHI/Config.hpp>
#include <RHI/Types.hpp>
#include <vulkan/vulkan.h>
#include <glm/glm.hpp>

namespace rhi::vulkan
{

struct SwapchainDescriptor
{
    Format              m_format;
    VkPresentModeKHR    m_presentMode;
    glm::ivec2          m_extent;
};

class RHI_API Swapchain
{
public:
    Swapchain(const SwapchainDescriptor& desc);
    ~Swapchain();

private:
    SwapchainDescriptor         m_descriptor;
    VkSwapchainKHR              m_swapchain;
    eastl::vector<VkImage>      m_images;
    eastl::vector<VkImageView>  m_imageViews;
};

}
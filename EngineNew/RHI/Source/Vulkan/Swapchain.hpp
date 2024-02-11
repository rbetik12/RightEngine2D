#pragma once

#include <RHI/Config.hpp>
#include <RHI/Types.hpp>
#include "Semaphore.hpp"
#include <vulkan/vulkan.h>
#include <glm/glm.hpp>

namespace rhi::vulkan
{

struct CommandBufferSync;

struct SwapchainDescriptor
{
    VkPresentModeKHR    m_presentMode;
    glm::ivec2          m_extent;
};

class RHI_API Swapchain
{
public:
    Swapchain(const SwapchainDescriptor& desc);
    ~Swapchain();

    uint32_t                    AcquireNextImage(VkDevice device, VkSemaphore presentSemaphore);
    inline VkImage              Image(uint32_t idx) const { RHI_ASSERT(idx < m_images.size()); return m_images[idx]; }
    inline VkImageView          ImageView(uint32_t idx) const { RHI_ASSERT(idx < m_imageViews.size()); return m_imageViews[idx]; }
    const SwapchainDescriptor&  Descriptor() const { return m_descriptor; }
    VkSwapchainKHR*             Raw() { return &m_swapchain; }

private:
    SwapchainDescriptor         m_descriptor;
    VkSwapchainKHR              m_swapchain;
    eastl::vector<VkImage>      m_images;
    eastl::vector<VkImageView>  m_imageViews;
};

}

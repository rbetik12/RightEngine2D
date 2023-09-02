#pragma once

#include "Surface.hpp"
#include "VulkanRenderingContext.hpp"
#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>

namespace RightEngine
{
    class VulkanSurface : public Surface
    {
    public:
        VulkanSurface(const std::shared_ptr<Window>& window,
                      const std::shared_ptr<RenderingContext>& context);
        ~VulkanSurface();

        VkSurfaceKHR GetSurface() const
        { return surface; }

    private:
        VkSurfaceKHR surface;
        std::weak_ptr<VulkanRenderingContext> ctx;

        void Init(const std::shared_ptr<Window>& window,
                  const std::shared_ptr<VulkanRenderingContext>& context);
    };
}

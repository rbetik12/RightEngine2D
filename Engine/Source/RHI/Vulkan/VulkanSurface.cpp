#include "VulkanSurface.hpp"
#include "Assert.hpp"

using namespace RightEngine;

VulkanSurface::VulkanSurface(const std::shared_ptr<Window>& window,
                             const std::shared_ptr<RenderingContext>& context) : Surface(window, context)
{
    const auto vulkanContext = std::static_pointer_cast<VulkanRenderingContext>(context);
    Init(window, vulkanContext);
    ctx = vulkanContext;
}

void VulkanSurface::Init(const std::shared_ptr<Window>& window,
                         const std::shared_ptr<VulkanRenderingContext>& context)
{
    if (glfwCreateWindowSurface(context->GetInstance(),
                                static_cast<GLFWwindow*>(window->GetNativeHandle()),
                                nullptr,
                                &surface) != VK_SUCCESS)
    {
        R_CORE_ASSERT(false, "Failed to create window surface!");
    }
}

VulkanSurface::~VulkanSurface()
{
    if (!ctx.expired())
    {
        vkDestroySurfaceKHR(ctx.lock()->GetInstance(), surface, nullptr);
    }
}

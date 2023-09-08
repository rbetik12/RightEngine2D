#include <cassert>
#include <iostream>
#include <EASTL/vector.h>
#include <RHI/IContext.hpp>
#include <RHI/Device.hpp>
#include <GLFW/glfw3.h>

int main(int argc, char* argv[])
{
    assert(glfwInit());

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    GLFWwindow* window = glfwCreateWindow(1920, 1080, "RHI Test", nullptr, nullptr);

    // Setup vulkan
    rhi::vulkan::VulkanInitContext ctx;
    ctx.m_surfaceConstructor = [&](VkInstance instance)
    {
        VkSurfaceKHR surface = nullptr;
        glfwCreateWindowSurface(instance, window, nullptr, &surface);
        return surface;
    };

    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    eastl::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
	extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

#ifdef R_APPLE
    extensions.push_back("VK_KHR_portability_enumeration");
    extensions.push_back("VK_KHR_get_physical_device_properties2");
#endif
    ctx.m_requiredExtensions = std::move(extensions);

    auto vulkanContext = rhi::vulkan::CreateContext(std::move(ctx));
    auto device = rhi::Device::Create(vulkanContext);

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
        glfwSwapBuffers(window);
    }

    glfwDestroyWindow(window);
	return 0;
}
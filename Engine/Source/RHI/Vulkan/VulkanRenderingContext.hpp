#pragma once

#include "RenderingContext.hpp"
#include <vulkan/vulkan.h>
#include <vector>

namespace RightEngine
{
    class VulkanRenderingContext : public RenderingContext
    {
    public:
        VulkanRenderingContext(const std::shared_ptr<Window>& window);

        virtual ~VulkanRenderingContext();

        VkInstance GetInstance() const
        { return instance; }

        std::vector<const char*> GetValidationLayers() const;

    private:
        VkInstance instance;
        VkDebugUtilsMessengerEXT debugMessenger;

        void Init();
        void SetupDebugMessenger();
    };
}
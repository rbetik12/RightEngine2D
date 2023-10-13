#include "VulkanContext.hpp"

#define VULKAN_LOG_VERBOSE 0

namespace rhi::vulkan
{

namespace
{

constexpr const bool C_ENABLE_VALIDATION_LAYERS = true;

const eastl::vector<const char*> C_VALIDATION_LAYERS =
{
    "VK_LAYER_KHRONOS_validation"
};

bool CheckValidationLayerSupport()
{
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    for (const auto layerName : C_VALIDATION_LAYERS)
    {
        bool layerFound = false;

        for (const auto& layerProperties : availableLayers)
        {
            if (strcmp(layerName, layerProperties.layerName) == 0)
            {
                layerFound = true;
                break;
            }
        }

        if (!layerFound)
        {
            return false;
        }
    }

    return true;
}

VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
											 VkDebugUtilsMessageTypeFlagsEXT messageType,
											 const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
											 void* pUserData)
{
#if VULKAN_LOG_VERBOSE
    if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT)
    {
        rhi::log::debug("[Vulkan]: {}", pCallbackData->pMessage);
    }

    if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT)
    {
        rhi::log::info("[Vulkan]: {}", pCallbackData->pMessage);
    }
#endif

    if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
    {
        rhi::log::warning("[Vulkan]: {}", pCallbackData->pMessage);
    }

    if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
    {
        rhi::log::error("[Vulkan]: {}", pCallbackData->pMessage);
    }

    return VK_FALSE;
}

void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo)
{
    createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity =
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT;
    createInfo.messageType =
        VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = DebugCallback;
}

} // namespace unnamed

VulkanContext::VulkanContext(VulkanInitContext&& ctx)
{
	vkEnumerateInstanceExtensionProperties(nullptr, &m_extensionAmount, nullptr);

    if (C_ENABLE_VALIDATION_LAYERS)
    {
        RHI_ASSERT(CheckValidationLayerSupport());
    }

    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Right Engine Editor";
#ifdef R_WIN32
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 3, 0);
    appInfo.engineVersion = VK_MAKE_VERSION(1, 3, 0);
    appInfo.apiVersion = VK_API_VERSION_1_3;
#elif R_APPLE
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;
#endif
    appInfo.pEngineName = "Right Engine";

    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;
#ifdef R_APPLE
    createInfo.flags = VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
#endif
    createInfo.enabledExtensionCount = static_cast<uint32_t>(ctx.m_requiredExtensions.size());
    createInfo.ppEnabledExtensionNames = ctx.m_requiredExtensions.data();

    if constexpr (C_ENABLE_VALIDATION_LAYERS)
    {
        VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
        createInfo.enabledLayerCount = static_cast<uint32_t>(C_VALIDATION_LAYERS.size());
        createInfo.ppEnabledLayerNames = C_VALIDATION_LAYERS.data();
        PopulateDebugMessengerCreateInfo(debugCreateInfo);
        createInfo.pNext = &debugCreateInfo;
    }
    else
    {
        createInfo.enabledLayerCount = 0;
        createInfo.pNext = nullptr;
    }

    if (vkCreateInstance(&createInfo, nullptr, &m_instance) != VK_SUCCESS)
    {
        RHI_ASSERT_WITH_MESSAGE(false, "Failed to create vulkan instance!");
    }

	//TODO: Add more info about current vulkan instance
	rhi::log::info("[Vulkan] Successfully initialized Vulkan API with {} extensions", m_extensionAmount);

    m_surface = ctx.m_surfaceConstructor(m_instance);
}

VulkanContext::~VulkanContext()
{
    vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
    vkDestroyInstance(m_instance, nullptr);
}

const eastl::vector<const char*>& VulkanContext::ValidationLayers() const
{
    if constexpr (C_ENABLE_VALIDATION_LAYERS)
    {
        return C_VALIDATION_LAYERS;
    }
    else
    {
        return {};
    }
}

} // namespace rhi::vulkan

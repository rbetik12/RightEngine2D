#include "VulkanDevice.hpp"
#include "Assert.hpp"
#include "VulkanRenderingContext.hpp"
#include "VulkanBuffer.hpp"
#include "VulkanShader.hpp"
#include "VulkanCommandBuffer.hpp"
#include "VulkanGraphicsPipeline.hpp"
#include "VulkanTexture.hpp"
#include "VulkanSampler.hpp"
#include <unordered_set>

using namespace RightEngine;

namespace
{
    const std::vector<const char*> deviceExtensions =
        {
            VK_KHR_SWAPCHAIN_EXTENSION_NAME,
#ifdef R_APPLE
            "VK_KHR_portability_subset",
#endif
        };

    std::weak_ptr<VulkanSurface> vulkanSurface;

    QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device)
    {
        QueueFamilyIndices indices;

        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

        int i = 0;
        for (const auto& queueFamily: queueFamilies)
        {
            if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
            {
                indices.graphicsFamily = i;
            }

            if (!vulkanSurface.expired())
            {
                VkBool32 presentSupport = false;
                auto surface = vulkanSurface.lock();
                vkGetPhysicalDeviceSurfaceSupportKHR(device,
                                                     i,
                                                     surface->GetSurface(),
                                                     &presentSupport);

                if (presentSupport)
                {
                    indices.presentFamily = i;
                }
            }

            if (indices.IsComplete())
            {
                break;
            }

            i++;
        }

        return indices;
    }

    bool CheckDeviceExtensionSupport(VkPhysicalDevice device)
    {
        uint32_t extensionCount;
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

        std::vector<VkExtensionProperties> availableExtensions(extensionCount);
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

        std::unordered_set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

        for (const auto& extension : availableExtensions)
        {
            requiredExtensions.erase(extension.extensionName);
        }

        return requiredExtensions.empty();
    }

    SwapchainSupportDetails QuerySwapchainSupport(VkPhysicalDevice device)
    {
        SwapchainSupportDetails details;

        if (vulkanSurface.expired())
        {
            R_CORE_ASSERT(false, "");
        }

        auto surface = vulkanSurface.lock()->GetSurface();

        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

        uint32_t formatCount;
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

        if (formatCount != 0)
        {
            details.formats.resize(formatCount);
            vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
        }

        uint32_t presentModeCount;
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);

        if (presentModeCount != 0)
        {
            details.presentModes.resize(presentModeCount);
            vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
        }

        return details;
    }

    bool IsDeviceSuitable(VkPhysicalDevice device)
    {
        QueueFamilyIndices indices = FindQueueFamilies(device);

        bool extensionsSupported = CheckDeviceExtensionSupport(device);

        bool swapchainAdequate = false;
        if (extensionsSupported)
        {
            SwapchainSupportDetails swapchainSupport = QuerySwapchainSupport(device);
            swapchainAdequate = !swapchainSupport.formats.empty() && !swapchainSupport.presentModes.empty();
        }

        VkPhysicalDeviceFeatures supportedFeatures;
        vkGetPhysicalDeviceFeatures(device, &supportedFeatures);

        VkPhysicalDeviceProperties deviceProps;
        vkGetPhysicalDeviceProperties(device, &deviceProps);

        return indices.IsComplete()
        && extensionsSupported
        && swapchainAdequate
        && supportedFeatures.samplerAnisotropy
        && deviceProps.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;
    }
}

VulkanDevice::VulkanDevice(const std::shared_ptr<RenderingContext>& context,
                           const std::shared_ptr<Surface>& surface) : Device(context, surface)
{
    auto ctx = std::static_pointer_cast<VulkanRenderingContext>(context);
    auto surf = std::static_pointer_cast<VulkanSurface>(surface);
    vulkanSurface = surf;
    Init(ctx);

    VkPhysicalDeviceProperties deviceProps;
    vkGetPhysicalDeviceProperties(physicalDevice, &deviceProps);

    properties.minUniformBufferOffsetAlignment = deviceProps.limits.minUniformBufferOffsetAlignment;
}

void VulkanDevice::Init(const std::shared_ptr<VulkanRenderingContext>& context)
{
    PickPhysicalDevice(context);
    CreateLogicalDevice(context);
    SetupDeviceQueues(context);
    SetupAllocator(context);
}

void VulkanDevice::PickPhysicalDevice(const std::shared_ptr<VulkanRenderingContext>& context)
{
    VkInstance instance = context->GetInstance();
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

    if (deviceCount == 0)
    {
        R_CORE_ASSERT(false, "");
    }

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

    for (const auto& device: devices)
    {
        if (IsDeviceSuitable(device))
        {
            physicalDevice = device;
            VkPhysicalDeviceProperties properties;
            vkGetPhysicalDeviceProperties(physicalDevice, &properties);
            R_CORE_INFO(R_VULKAN_LOG("Initialized physical device: {0}"), properties.deviceName);
            break;
        }
    }

    if (physicalDevice == VK_NULL_HANDLE)
    {
        R_CORE_ASSERT(false, "");
    }
}

void VulkanDevice::CreateLogicalDevice(const std::shared_ptr<VulkanRenderingContext>& context)
{
    QueueFamilyIndices indices = ::FindQueueFamilies(physicalDevice);
    R_CORE_ASSERT(indices.IsComplete(), "");

    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    std::unordered_set<uint32_t> uniqueQueueFamilies = { indices.graphicsFamily.value(),
                                                         indices.presentFamily.value() };

    float queuePriority = 1.0f;
    for (uint32_t queueFamily: uniqueQueueFamilies)
    {
        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueFamily;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
    }

    VkPhysicalDeviceFeatures deviceFeatures{};
    deviceFeatures.samplerAnisotropy = VK_TRUE;

    VkDeviceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.pQueueCreateInfos = queueCreateInfos.data();
    createInfo.queueCreateInfoCount = queueCreateInfos.size();
    createInfo.pEnabledFeatures = &deviceFeatures;
    createInfo.enabledExtensionCount = deviceExtensions.size();
    createInfo.ppEnabledExtensionNames = deviceExtensions.data();

    const auto validationLayers = context->GetValidationLayers();

    if (validationLayers.size() > 0)
    {
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();
    }
    else
    {
        createInfo.enabledLayerCount = 0;
    }

    if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS)
    {
        R_CORE_ASSERT(false, "Failed to create logical device!");
    }
}

void VulkanDevice::SetupDeviceQueues(const std::shared_ptr<VulkanRenderingContext>& context)
{
    const auto indices = ::FindQueueFamilies(physicalDevice);
    R_CORE_ASSERT(indices.IsComplete(), "");
    vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0, &graphicsQueue);
    vkGetDeviceQueue(device, indices.presentFamily.value(), 0, &presentQueue);
}

SwapchainSupportDetails VulkanDevice::GetSwapchainSupportDetails() const
{
    return QuerySwapchainSupport(physicalDevice);
}

QueueFamilyIndices VulkanDevice::FindQueueFamilies() const
{
    return ::FindQueueFamilies(physicalDevice);
}

VulkanDevice::~VulkanDevice()
{
    vmaDestroyAllocator(allocator);
    vkDestroyDevice(device, nullptr);
}

std::shared_ptr<Buffer> VulkanDevice::CreateBuffer(const BufferDescriptor& bufferDescriptor, const void* data)
{
    return std::make_shared<VulkanBuffer>(shared_from_this(), bufferDescriptor, data);
}

std::shared_ptr<Shader> VulkanDevice::CreateShader(const ShaderProgramDescriptor& shaderProgramDescriptor)
{
    return std::make_shared<VulkanShader>(shared_from_this(), shaderProgramDescriptor);
}

std::shared_ptr<CommandBuffer> VulkanDevice::CreateCommandBuffer(const CommandBufferDescriptor& descriptor)
{
    return std::make_shared<VulkanCommandBuffer>(shared_from_this(), descriptor);
}

std::shared_ptr<GraphicsPipeline> VulkanDevice::CreateGraphicsPipeline(const GraphicsPipelineDescriptor& descriptor,
                                     const RenderPassDescriptor& renderPassDescriptor)
{
    return std::make_shared<VulkanGraphicsPipeline>(descriptor, renderPassDescriptor);
}

std::shared_ptr<Texture> VulkanDevice::CreateTexture(const TextureDescriptor& descriptor,
                                                     const std::vector<uint8_t>& data)
{
    return std::make_shared<VulkanTexture>(shared_from_this(), descriptor, data);
}

std::shared_ptr<Sampler> VulkanDevice::CreateSampler(const SamplerDescriptor& descriptor)
{
    return std::make_shared<VulkanSampler>(shared_from_this(), descriptor);
}

void VulkanDevice::SetupAllocator(const std::shared_ptr<VulkanRenderingContext>& context)
{
    VmaAllocatorCreateInfo allocatorInfo = {};
    allocatorInfo.physicalDevice = physicalDevice;
    allocatorInfo.device = device;
    allocatorInfo.instance = context->GetInstance();
    vmaCreateAllocator(&allocatorInfo, &allocator);
}

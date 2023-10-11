#include "VulkanDevice.hpp"
#include "VulkanShaderCompiler.hpp"
#include "VulkanBuffer.hpp"
#include "VulkanShader.hpp"
#include "VulkanSampler.hpp"
#include <optional>

namespace rhi::vulkan
{

namespace
{

const eastl::array<const char*> C_DEVICE_EXTENSIONS =
{
	VK_KHR_SWAPCHAIN_EXTENSION_NAME,
#ifdef R_APPLE
		"VK_KHR_portability_subset",
#endif
};

struct QueueFamilyIndices
{
	std::optional<uint32_t> graphicsFamily;
	std::optional<uint32_t> presentFamily;

	bool IsComplete() const
	{
		return graphicsFamily.has_value() && presentFamily.has_value();
	}
};

bool CheckDeviceExtensionSupport(VkPhysicalDevice device)
{
	uint32_t extensionCount;
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

	eastl::vector<VkExtensionProperties> availableExtensions(extensionCount);
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

	eastl::unordered_set<std::string> requiredExtensions(C_DEVICE_EXTENSIONS.begin(), C_DEVICE_EXTENSIONS.end());

	for (const auto& extension : availableExtensions)
	{
		requiredExtensions.erase(extension.extensionName);
	}

	return requiredExtensions.empty();
}

std::string_view PhysicalDeviceTypeToString(VkPhysicalDeviceType type)
{
	switch (type)
	{
		case VK_PHYSICAL_DEVICE_TYPE_OTHER: return "Other";
		case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU: return "Integrated GPU";
		case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU: return "Discrete GPU";
		case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU: return "Virtual GPU";
		case VK_PHYSICAL_DEVICE_TYPE_CPU: return "CPU";
		default:
		{
			RHI_ASSERT(false);
			return "";
		}
	}
}

QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface)
{
	QueueFamilyIndices indices;

	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

	eastl::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

	int i = 0;
	for (const auto& queueFamily : queueFamilies)
	{
		if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
		{
			indices.graphicsFamily = i;
		}

		VkBool32 presentSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);

		if (presentSupport)
		{
			indices.presentFamily = i;
		}

		if (indices.IsComplete())
		{
			break;
		}

		i++;
	}

	return indices;
}

SwapchainSupportDetails QuerySwapchainSupport(VkPhysicalDevice device, VkSurfaceKHR surface)
{
	SwapchainSupportDetails details;
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.m_capabilities);

	uint32_t formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

	if (formatCount != 0)
	{
		details.m_formats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.m_formats.data());
	}

	uint32_t presentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);

	if (presentModeCount != 0)
	{
		details.m_presentModes.resize(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.m_presentModes.data());
	}

	return details;
}

bool IsDeviceSuitable(VkPhysicalDevice device, VkSurfaceKHR surface)
{
	QueueFamilyIndices indices = FindQueueFamilies(device, surface);

	bool extensionsSupported = CheckDeviceExtensionSupport(device);

	bool swapchainAdequate = false;
	if (extensionsSupported)
	{
		SwapchainSupportDetails swapchainSupport = QuerySwapchainSupport(device, surface);
		swapchainAdequate = !swapchainSupport.m_formats.empty() && !swapchainSupport.m_presentModes.empty();
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

} // namespace unnamed

VulkanDevice::VulkanDevice(const std::shared_ptr<VulkanContext>& context)
{
	PickPhysicalDevice(context);
	CreateLogicalDevice(context);
	SetupDeviceQueues(context);
	FillProperties();
	SetupAllocator(context);
	SetupCommandPool(context);
	FillSwapchainSupportDetails(context);
	s_ctx.m_instance = this;
}

VulkanDevice::~VulkanDevice()
{
	vkDestroyCommandPool(s_ctx.m_device, m_commandPool, nullptr);
	vmaDestroyAllocator(s_ctx.m_allocator);
	vkDestroyDevice(s_ctx.m_device, nullptr);
}

std::shared_ptr<ShaderCompiler> VulkanDevice::CreateShaderCompiler(const ShaderCompiler::Options& options)
{
	return std::make_shared<VulkanShaderCompiler>(options);
}

std::shared_ptr<Buffer> VulkanDevice::CreateBuffer(const BufferDescriptor& desc, const void* data)
{
	return std::make_shared<VulkanBuffer>(desc, data);
}

std::shared_ptr<Shader> VulkanDevice::CreateShader(const ShaderDescriptor& desc)
{
	return std::make_shared<VulkanShader>(desc);
}

std::shared_ptr<Sampler> VulkanDevice::CreateSampler(const SamplerDescriptor& desc)
{
	return std::make_shared<VulkanSampler>(desc);
}

void VulkanDevice::FillSwapchainSupportDetails(const std::shared_ptr<VulkanContext>& context)
{
	SwapchainSupportDetails details;

	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(s_ctx.m_physicalDevice, context->Surface(), &details.m_capabilities);

	uint32_t formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(s_ctx.m_physicalDevice, context->Surface(), &formatCount, nullptr);

	if (formatCount != 0)
	{
		details.m_formats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(s_ctx.m_physicalDevice, context->Surface(), &formatCount, details.m_formats.data());
	}

	uint32_t presentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(s_ctx.m_physicalDevice, context->Surface(), &presentModeCount, nullptr);

	if (presentModeCount != 0)
	{
		details.m_presentModes.resize(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(s_ctx.m_physicalDevice, context->Surface(), &presentModeCount, details.m_presentModes.data());
	}

	m_swapchainDetails = std::move(details);
}

Fence VulkanDevice::Execute(CommandBuffer buffer)
{
	const auto cmd = buffer.Raw();

	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &cmd;

	Fence fence;
	vkQueueSubmit(m_graphicsQueue, 1, &submitInfo, fence.Raw());

	return fence;
}

void VulkanDevice::PickPhysicalDevice(const std::shared_ptr<VulkanContext>& context)
{
	VkInstance instance = context->Instance();
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

	RHI_ASSERT_WITH_MESSAGE(deviceCount != 0, "No devices supporting vulkan was found!");

	eastl::vector<VkPhysicalDevice> devices(deviceCount);
	vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

	for (const auto& device : devices)
	{
		if (IsDeviceSuitable(device, context->Surface()))
		{
			s_ctx.m_physicalDevice = device;
			VkPhysicalDeviceProperties properties;
			vkGetPhysicalDeviceProperties(s_ctx.m_physicalDevice, &properties);
			rhi::log::info("[Vulkan] Successfully initialized physical device: {}. Device type: {}. Device API version: {}.{}.{}", properties.deviceName, 
																											PhysicalDeviceTypeToString(properties.deviceType),
																											VK_API_VERSION_MAJOR(properties.apiVersion),
																											VK_API_VERSION_MINOR(properties.apiVersion),
																											VK_API_VERSION_PATCH(properties.apiVersion));
			break;
		}
	}

	RHI_ASSERT_WITH_MESSAGE(s_ctx.m_physicalDevice != nullptr, "No suitable physical device was found!");
}

void VulkanDevice::CreateLogicalDevice(const std::shared_ptr<VulkanContext>& context)
{
	QueueFamilyIndices indices = FindQueueFamilies(s_ctx.m_physicalDevice, context->Surface());
	RHI_ASSERT(indices.IsComplete());

	eastl::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	eastl::unordered_set<uint32_t> uniqueQueueFamilies = {
															indices.graphicsFamily.value(),
															indices.presentFamily.value()
														 };

	float queuePriority = 1.0f;
	for (uint32_t queueFamily : uniqueQueueFamilies)
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
	createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
	createInfo.pEnabledFeatures = &deviceFeatures;
	createInfo.enabledExtensionCount = static_cast<uint32_t>(C_DEVICE_EXTENSIONS.size());
	createInfo.ppEnabledExtensionNames = C_DEVICE_EXTENSIONS.data();

	const auto validationLayers = context->ValidationLayers();

	if (validationLayers.size() > 0)
	{
		createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
		createInfo.ppEnabledLayerNames = validationLayers.data();
	}
	else
	{
		createInfo.enabledLayerCount = 0;
	}

	RHI_ASSERT_WITH_MESSAGE(vkCreateDevice(s_ctx.m_physicalDevice,
										   &createInfo, 
										   nullptr, 
										   &s_ctx.m_device) == VK_SUCCESS, "Failed to create logical device!");
}

void VulkanDevice::SetupDeviceQueues(const std::shared_ptr<VulkanContext>& context)
{
	const auto indices = FindQueueFamilies(s_ctx.m_physicalDevice, context->Surface());
	RHI_ASSERT(indices.IsComplete());
	vkGetDeviceQueue(s_ctx.m_device, indices.graphicsFamily.value(), 0, &m_graphicsQueue);
	vkGetDeviceQueue(s_ctx.m_device, indices.presentFamily.value(), 0, &m_presentQueue);
}

void VulkanDevice::SetupAllocator(const std::shared_ptr<VulkanContext>& context)
{
	VmaAllocatorCreateInfo allocatorInfo{};
	allocatorInfo.physicalDevice = s_ctx.m_physicalDevice;
	allocatorInfo.device = s_ctx.m_device;
	allocatorInfo.instance = context->Instance();
	vmaCreateAllocator(&allocatorInfo, &s_ctx.m_allocator);
}

void VulkanDevice::SetupCommandPool(const std::shared_ptr<VulkanContext>& context)
{
	VkCommandPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	poolInfo.queueFamilyIndex = FindQueueFamilies(s_ctx.m_physicalDevice, context->Surface()).graphicsFamily.value();

	RHI_ASSERT(vkCreateCommandPool(s_ctx.m_device, &poolInfo, nullptr, &m_commandPool) != VK_SUCCESS);
}

void VulkanDevice::FillProperties()
{
	VkPhysicalDeviceProperties deviceProps;
	vkGetPhysicalDeviceProperties(s_ctx.m_physicalDevice, &deviceProps);

	Properties& properties = s_ctx.m_properties;

	properties.m_minUniformBufferOffsetAlignment = deviceProps.limits.minUniformBufferOffsetAlignment;
	properties.m_maxSamplerAnisotropy = deviceProps.limits.maxSamplerAnisotropy;
}

}

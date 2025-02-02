#include "VulkanDevice.hpp"
#include "VulkanShaderCompiler.hpp"
#include "VulkanBuffer.hpp"
#include "VulkanShader.hpp"
#include "VulkanSampler.hpp"
#include "VulkanTexture.hpp"
#include "VulkanRenderPass.hpp"
#include "VulkanPipeline.hpp"
#include "VulkanHelpers.hpp"
#include "VulkanGPUMaterial.hpp"
#include "VulkanComputeState.hpp"
#include <Core/Profiling.hpp>
#include <vk-tools/VulkanTools.h>
#include <optional>

namespace rhi::vulkan
{

namespace
{

const eastl::array<const char*, 2> C_DEVICE_EXTENSIONS =
{
    VK_KHR_SWAPCHAIN_EXTENSION_NAME,
    VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME,
#ifdef R_APPLE
        "VK_KHR_portability_subset",
#endif
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
        if ((queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) && (queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT))
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
    s_ctx.m_instance = this;
    m_context = context;

    PickPhysicalDevice(context);
    CreateLogicalDevice(context);
    SetupDeviceQueues(context);
    FillProperties();
    SetupAllocator(context);
    SetupCommandPool(context);

    m_cmdBuffers.resize(s_ctx.m_instance->m_parameters.m_framesInFlight);
    m_computeCmdBuffers.resize(s_ctx.m_instance->m_parameters.m_framesInFlight);
    m_fences.resize(s_ctx.m_instance->m_parameters.m_framesInFlight);
    m_computeFences.resize(s_ctx.m_instance->m_parameters.m_framesInFlight);
    m_renderSemaphores.resize(s_ctx.m_instance->m_parameters.m_framesInFlight);
    m_presentSemaphores.resize(s_ctx.m_instance->m_parameters.m_framesInFlight);
    m_computeSemaphores.resize(s_ctx.m_instance->m_parameters.m_framesInFlight);

    for (uint32_t i = 0; i < s_ctx.m_instance->m_parameters.m_framesInFlight; i++)
    {
        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = s_ctx.m_instance->CommandPool();
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = 1;
        RHI_ASSERT(vkAllocateCommandBuffers(VulkanDevice::s_ctx.m_device, &allocInfo, &m_cmdBuffers[i]) == VK_SUCCESS);
        RHI_ASSERT(vkAllocateCommandBuffers(VulkanDevice::s_ctx.m_device, &allocInfo, &m_computeCmdBuffers[i]) == VK_SUCCESS);

        VkFenceCreateInfo fenceInfo{};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
        RHI_ASSERT(vkCreateFence(VulkanDevice::s_ctx.m_device, &fenceInfo, nullptr, &m_fences[i]) == VK_SUCCESS);
        RHI_ASSERT(vkCreateFence(VulkanDevice::s_ctx.m_device, &fenceInfo, nullptr, &m_computeFences[i]) == VK_SUCCESS);

        VkSemaphoreCreateInfo semaphoreInfo{};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        RHI_ASSERT(vkCreateSemaphore(VulkanDevice::s_ctx.m_device, &semaphoreInfo, nullptr, &m_presentSemaphores[i]) == VK_SUCCESS);
        RHI_ASSERT(vkCreateSemaphore(VulkanDevice::s_ctx.m_device, &semaphoreInfo, nullptr, &m_renderSemaphores[i]) == VK_SUCCESS);
        RHI_ASSERT(vkCreateSemaphore(VulkanDevice::s_ctx.m_device, &semaphoreInfo, nullptr, &m_computeSemaphores[i]) == VK_SUCCESS);
    }
}

VulkanDevice::~VulkanDevice()
{
    vkDeviceWaitIdle(s_ctx.m_device);
    VulkanGPUMaterial::Destroy();
    m_swapchain.reset();

    for (uint32_t i = 0; i < s_ctx.m_instance->m_parameters.m_framesInFlight; i++)
    {
        vkDestroyFence(s_ctx.m_device, m_fences[i], nullptr);
        vkDestroySemaphore(s_ctx.m_device, m_presentSemaphores[i], nullptr);
        vkDestroySemaphore(s_ctx.m_device, m_renderSemaphores[i], nullptr);
    }

    vkDestroyCommandPool(s_ctx.m_device, m_commandPool, nullptr);
    vmaDestroyAllocator(s_ctx.m_allocator);
    vkDestroyDevice(s_ctx.m_device, nullptr);
}

std::shared_ptr<ShaderCompiler> VulkanDevice::CreateShaderCompiler(const ShaderCompiler::Options& options)
{
    static bool created = false;

    RHI_ASSERT_WITH_MESSAGE(!created, "Currently engine supports only one instance of shader compiler!");

    if (!created)
    {
        created = true;
    }
    return std::make_shared<VulkanShaderCompiler>(options);
}

std::shared_ptr<Buffer> VulkanDevice::CreateBuffer(const BufferDescriptor& desc, const void* data)
{
    RHI_ASSERT(!desc.m_name.empty());
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

std::shared_ptr<Texture> VulkanDevice::CreateTexture(const TextureDescriptor& desc, const std::shared_ptr<Sampler>& sampler, const void* data)
{
    return std::make_shared<VulkanTexture>(desc, sampler, data);
}

std::shared_ptr<RenderPass> VulkanDevice::CreateRenderPass(const RenderPassDescriptor& desc)
{
    return std::make_shared<VulkanRenderPass>(desc);
}

std::shared_ptr<Pipeline> VulkanDevice::CreatePipeline(const PipelineDescriptor& desc)
{
    return std::make_shared<VulkanPipeline>(desc);
}

std::shared_ptr<GPUMaterial> VulkanDevice::CreateGPUMaterial(const std::shared_ptr<Shader>& shader)
{
    const auto vkShader = std::static_pointer_cast<VulkanShader>(shader);
    return std::make_shared<VulkanGPUMaterial>(vkShader);
}

void VulkanDevice::BeginFrame()
{
    PROFILER_CPU_ZONE;

    if (m_isSwapchainDirty)
    {
        vkDeviceWaitIdle(s_ctx.m_device);
        m_swapchain.reset();
        m_context->RecreateSurface();
        FillSwapchainSupportDetails(m_context);

        SwapchainDescriptor descriptor{};
        descriptor.m_extent = m_presentExtent;
        descriptor.m_presentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
        descriptor.m_surface = m_context->Surface();

        m_swapchain = std::make_unique<Swapchain>(descriptor);
        m_isSwapchainDirty = false;
    }

    RHI_ASSERT(m_presentExtent != glm::ivec2());

    m_frameIndex += 1;
    m_currentCmdBufferIndex = m_frameIndex % m_cmdBuffers.size();

    auto& cmdBuffer = m_cmdBuffers[m_currentCmdBufferIndex];
    RHI_ASSERT(vkResetCommandBuffer(cmdBuffer, 0) == VK_SUCCESS);
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    RHI_ASSERT(vkBeginCommandBuffer(cmdBuffer, &beginInfo) == VK_SUCCESS);

    auto& presentSemaphore = m_presentSemaphores[m_currentCmdBufferIndex];

    m_swapchainImageIndex = m_swapchain->AcquireNextImage(s_ctx.m_device, presentSemaphore);
}

void VulkanDevice::EndFrame()
{
    PROFILER_CPU_ZONE;

    auto& cmdBuffer = m_cmdBuffers[m_currentCmdBufferIndex];
    RHI_ASSERT(vkEndCommandBuffer(cmdBuffer) == VK_SUCCESS);

    VkPipelineStageFlags waitStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.pWaitDstStageMask = &waitStageMask;
    submitInfo.pWaitSemaphores = &m_presentSemaphores[m_currentCmdBufferIndex];
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = &m_renderSemaphores[m_currentCmdBufferIndex];
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pCommandBuffers = &cmdBuffer;
    submitInfo.commandBufferCount = 1;

    RHI_ASSERT(vkResetFences(s_ctx.m_device, 1, &m_fences[m_currentCmdBufferIndex]) == VK_SUCCESS);
    RHI_ASSERT(vkQueueSubmit(m_graphicsQueue, 1, &submitInfo, m_fences[m_currentCmdBufferIndex]) == VK_SUCCESS);
}

void VulkanDevice::BeginComputePipeline(const std::shared_ptr<Pipeline>& pipeline)
{
    RHI_ASSERT(pipeline->Descriptor().m_compute);

    auto& cmdBuffer = m_cmdBuffers[m_currentCmdBufferIndex];

    for (const auto& texture : pipeline->Descriptor().m_computePass->m_storageTextures)
    {
        auto vkTexture = std::static_pointer_cast<VulkanTexture>(texture);
        vkTexture->ChangeImageLayout(cmdBuffer, vkTexture->Layout(), VK_IMAGE_LAYOUT_GENERAL);
        m_computeTexturesToReset.emplace_back(vkTexture);
    }

    const auto vkPipeline = std::static_pointer_cast<VulkanPipeline>(pipeline);
    vkCmdBindPipeline(cmdBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, vkPipeline->GetPipeline());
}

void VulkanDevice::EndComputePipeline(const std::shared_ptr<Pipeline>& pipeline)
{
    RHI_ASSERT(pipeline->Descriptor().m_compute);

    auto& cmdBuffer = m_cmdBuffers[m_currentCmdBufferIndex];

    for (const auto& texture : m_computeTexturesToReset)
    {
        auto vkTexture = std::static_pointer_cast<VulkanTexture>(texture);
        vkTexture->ChangeImageLayout(cmdBuffer, vkTexture->Layout(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    }
    m_computeTexturesToReset.clear();
}

std::shared_ptr<ComputeState> VulkanDevice::BeginComputePipelineImmediate(const std::shared_ptr<Pipeline>& pipeline)
{
    const auto state = std::make_shared<VulkanComputeState>();

    RHI_ASSERT(pipeline->Descriptor().m_compute);

    const auto cmdBuffer = state->m_cmdBuffer.Raw();

    for (const auto& texture : pipeline->Descriptor().m_computePass->m_storageTextures)
    {
        auto vkTexture = std::static_pointer_cast<VulkanTexture>(texture);
        vkTexture->ChangeImageLayout(cmdBuffer, vkTexture->Layout(), VK_IMAGE_LAYOUT_GENERAL);
        m_computeTexturesToReset.emplace_back(vkTexture);
    }

    const auto vkPipeline = std::static_pointer_cast<VulkanPipeline>(pipeline);
    vkCmdBindPipeline(cmdBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, vkPipeline->GetPipeline());

    return state;
}

void VulkanDevice::EndComputePipeline(const std::shared_ptr<Pipeline>& pipeline,
    const std::shared_ptr<ComputeState>& state)
{
    RHI_ASSERT(pipeline->Descriptor().m_compute);
    RHI_ASSERT(state);

    const auto vkState = std::static_pointer_cast<VulkanComputeState>(state);
    const auto cmdBuffer = vkState->m_cmdBuffer.Raw();

    for (const auto& texture : m_computeTexturesToReset)
    {
        auto vkTexture = std::static_pointer_cast<VulkanTexture>(texture);
        vkTexture->ChangeImageLayout(cmdBuffer, vkTexture->Layout(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    }
    m_computeTexturesToReset.clear();

    vkState->m_cmdBuffer.End();
    Execute(vkState->m_cmdBuffer)->Wait();
}

void VulkanDevice::PushConstantComputeImmediate(const void* data, uint32_t size, const std::shared_ptr<Pipeline>& pipeline, const std::shared_ptr<ComputeState>& state)
{
    const auto vkState = std::static_pointer_cast<VulkanComputeState>(state);
    vkCmdPushConstants(vkState->m_cmdBuffer.Raw(),
        std::static_pointer_cast<VulkanPipeline>(pipeline)->Layout(),
        VK_SHADER_STAGE_COMPUTE_BIT,
        0,
        size,
        data);
}

void VulkanDevice::Present()
{
    VkResult result;
    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.pNext = nullptr;
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = m_swapchain->Raw();
    presentInfo.pImageIndices = &m_swapchainImageIndex;

    presentInfo.pWaitSemaphores = &m_renderSemaphores[m_currentCmdBufferIndex];
    presentInfo.waitSemaphoreCount = 1;
    result = vkQueuePresentKHR(m_presentQueue, &presentInfo);

    if (result != VK_SUCCESS)
    {
        if (result == VK_ERROR_OUT_OF_DATE_KHR)
        {
            m_isSwapchainDirty = true;
        }
        else
        {
            RHI_ASSERT(false);
        }
    }

    const uint32_t nextFrameIndex = (m_frameIndex + 1) % s_ctx.m_instance->m_parameters.m_framesInFlight;
    RHI_ASSERT(vkWaitForFences(s_ctx.m_device, 1, &m_fences[nextFrameIndex], VK_TRUE, UINT64_MAX) == VK_SUCCESS);
}

void VulkanDevice::BeginPipeline(const std::shared_ptr<Pipeline>& pipeline)
{
    PROFILER_CPU_ZONE;

    RHI_ASSERT(!pipeline->Descriptor().m_compute);

    auto& cmdBuffer = m_cmdBuffers[m_currentCmdBufferIndex];

    const auto& renderpass = std::static_pointer_cast<VulkanRenderPass>(pipeline->Descriptor().m_pass);

    VkRenderingInfoKHR renderingInfo{};
    renderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO_KHR;
    renderingInfo.renderArea = helpers::Rect(renderpass->Descriptor().m_extent);

    if (pipeline->Descriptor().m_offscreen)
    {
        renderingInfo.layerCount = 1;
        renderingInfo.colorAttachmentCount = static_cast<uint32_t>(renderpass->Descriptor().m_colorAttachments.size());
        renderingInfo.pColorAttachments = renderpass->ColorAttachments().data();

        if (renderpass->Descriptor().m_depthStencilAttachment.m_texture != nullptr)
        {
            renderingInfo.pDepthAttachment = &renderpass->DepthAttachment();
            renderingInfo.pStencilAttachment = &renderpass->DepthAttachment();
        }

        for (const auto& texture : renderpass->Descriptor().m_colorAttachments)
        {
            auto vkTexture = std::static_pointer_cast<VulkanTexture>(texture.m_texture);
            vkTexture->ChangeImageLayout(cmdBuffer, vkTexture->Layout(), VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
            m_texturesToReset.emplace_back(vkTexture);
        }

        vkCmdBeginRendering(cmdBuffer, &renderingInfo);
    }
    else
    {
        renderingInfo.layerCount = 1;

        VkClearValue cv;
        cv.depthStencil.stencil = 0;
        cv.depthStencil.depth = 1.0f;
        cv.color = { 0.0f, 0.0f, 0.0f, 1.0f };

        VkRenderingAttachmentInfoKHR colorAttachment{};
        colorAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR;
        colorAttachment.imageView = m_swapchain->ImageView(m_swapchainImageIndex);
        colorAttachment.clearValue = cv;
        colorAttachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

        renderingInfo.colorAttachmentCount = 1;
        renderingInfo.pColorAttachments = &colorAttachment;

        VkImageSubresourceRange srcSubRange{};
        srcSubRange.layerCount = 1;
        srcSubRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        srcSubRange.baseMipLevel = 0;
        srcSubRange.levelCount = 1;

        vks::tools::setImageLayout(cmdBuffer,
            m_swapchain->Image(m_swapchainImageIndex),
            VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
            VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            srcSubRange);

        vkCmdBeginRendering(cmdBuffer, &renderingInfo);
    }

    const auto vkPipeline = std::static_pointer_cast<VulkanPipeline>(pipeline);
    vkCmdBindPipeline(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vkPipeline->GetPipeline());

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(pipeline->Descriptor().m_pass->Descriptor().m_extent.x);
    viewport.height = static_cast<float>(pipeline->Descriptor().m_pass->Descriptor().m_extent.y);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor{};
    scissor.offset = { 0, 0 };
    scissor.extent = helpers::Extent(pipeline->Descriptor().m_pass->Descriptor().m_extent);

    vkCmdSetViewport(cmdBuffer, 0, 1, &viewport);
    vkCmdSetScissor(cmdBuffer, 0, 1, &scissor);
}

void VulkanDevice::EndPipeline(const std::shared_ptr<Pipeline>& pipeline)
{
    PROFILER_CPU_ZONE;

    RHI_ASSERT(!pipeline->Descriptor().m_compute);

    auto& cmdBuffer = m_cmdBuffers[m_currentCmdBufferIndex];
    vkCmdEndRendering(cmdBuffer);

    for (auto& texture : m_texturesToReset)
    {
        texture->ChangeImageLayout(cmdBuffer, texture->Layout(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    }
    m_texturesToReset.clear();

    if (!pipeline->Descriptor().m_offscreen)
    {
        VkImageSubresourceRange srcSubRange{};
        srcSubRange.layerCount = 1;
        srcSubRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        srcSubRange.baseMipLevel = 0;
        srcSubRange.levelCount = 1;

        vks::tools::setImageLayout(cmdBuffer,
            m_swapchain->Image(m_swapchainImageIndex),
            VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
            srcSubRange);
    }
}

void VulkanDevice::Draw(const std::shared_ptr<Buffer>& buffer, uint32_t vertexCount, uint32_t instanceCount)
{
    auto& cmdBuffer = m_cmdBuffers[m_currentCmdBufferIndex];
    VkBuffer vertexBuffers[] = { std::static_pointer_cast<VulkanBuffer>(buffer)->Raw() };
    VkDeviceSize offsets[] = { 0 };

    vkCmdBindVertexBuffers(cmdBuffer, 0, 1, vertexBuffers, offsets);
    vkCmdDraw(cmdBuffer, vertexCount, instanceCount, 0, 0);
}

void VulkanDevice::Draw(const std::shared_ptr<Buffer>& vb, const std::shared_ptr<Buffer>& ib, uint32_t indexCount,
    uint32_t instanceCount)
{
    RHI_ASSERT(vb->Descriptor().m_type == BufferType::VERTEX);
    RHI_ASSERT(ib->Descriptor().m_type == BufferType::INDEX);

    const auto vkVertexBuffer = std::static_pointer_cast<VulkanBuffer>(vb);
    const auto vkIndexBuffer = std::static_pointer_cast<VulkanBuffer>(ib);
    VkBuffer vertexBuffers[] = { vkVertexBuffer->Raw() };
    VkDeviceSize offsets[] = { 0 };

    auto& cmdBuffer = m_cmdBuffers[m_currentCmdBufferIndex];

    vkCmdBindVertexBuffers(cmdBuffer, 0, 1, vertexBuffers, offsets);
    vkCmdBindIndexBuffer(cmdBuffer, vkIndexBuffer->Raw(), 0, VK_INDEX_TYPE_UINT32);

    vkCmdDrawIndexed(cmdBuffer,
        indexCount,
        instanceCount,
        0,
        0,
        0);
}

void VulkanDevice::Dispatch(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ)
{
    auto& cmdBuffer = m_cmdBuffers[m_currentCmdBufferIndex];

    vkCmdDispatch(cmdBuffer, groupCountX, groupCountY, groupCountZ);
}

void VulkanDevice::Dispatch(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ,
    const std::shared_ptr<ComputeState>& state)
{
    RHI_ASSERT(state);

    const auto vkState = std::static_pointer_cast<VulkanComputeState>(state);
    vkCmdDispatch(vkState->m_cmdBuffer.Raw(), groupCountX, groupCountY, groupCountZ);
}

void VulkanDevice::BindGPUMaterial(const std::shared_ptr<GPUMaterial>& material, const std::shared_ptr<Pipeline>& pipeline)
{
    const auto vkPipeline = std::static_pointer_cast<VulkanPipeline>(pipeline);
    const auto vkMaterial = std::static_pointer_cast<VulkanGPUMaterial>(material);
    const auto descSet = vkMaterial->DescriptorSet();

    if (vkPipeline->Descriptor().m_compute)
    {
        vkCmdBindDescriptorSets(m_cmdBuffers[m_currentCmdBufferIndex],
            VK_PIPELINE_BIND_POINT_COMPUTE,
            vkPipeline->Layout(),
            0, 1,
            &descSet
            , 0, nullptr);
    }
    else
    {
        vkCmdBindDescriptorSets(m_cmdBuffers[m_currentCmdBufferIndex],
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            vkPipeline->Layout(),
            0, 1,
            &descSet
            , 0, nullptr);
    }
}

void VulkanDevice::BindGPUMaterial(const std::shared_ptr<GPUMaterial>& material,
    const std::shared_ptr<Pipeline>& pipeline, const std::shared_ptr<ComputeState>& state)
{
    RHI_ASSERT(state);

    const auto vkPipeline = std::static_pointer_cast<VulkanPipeline>(pipeline);
    const auto vkMaterial = std::static_pointer_cast<VulkanGPUMaterial>(material);
    const auto descSet = vkMaterial->DescriptorSet();
    const auto vkState = std::static_pointer_cast<VulkanComputeState>(state);

    if (vkPipeline->Descriptor().m_compute)
    {
        vkCmdBindDescriptorSets(vkState->m_cmdBuffer.Raw(),
            VK_PIPELINE_BIND_POINT_COMPUTE,
            vkPipeline->Layout(),
            0, 1,
            &descSet
            , 0, nullptr);
    }
    else
    {
        RHI_ASSERT(false);
    }
}

void VulkanDevice::PushConstant(const void* data, uint32_t size, const std::shared_ptr<Pipeline>& pipeline)
{
    vkCmdPushConstants(m_cmdBuffers[m_currentCmdBufferIndex],
        std::static_pointer_cast<VulkanPipeline>(pipeline)->Layout(),
        VK_SHADER_STAGE_VERTEX_BIT,
        0,
        size,
        data);
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

QueueFamilyIndices VulkanDevice::FindQueueFamilies() const
{
    return vulkan::FindQueueFamilies(s_ctx.m_physicalDevice, m_context->Surface());
}

std::shared_ptr<Fence> VulkanDevice::Execute(CommandBuffer buffer)
{
    const auto cmd = buffer.Raw();

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &cmd;

    auto fence = std::make_shared<Fence>(true);
    fence->Reset();
    vkQueueSubmit(m_graphicsQueue, 1, &submitInfo, *fence->Raw());

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
    QueueFamilyIndices indices = vulkan::FindQueueFamilies(s_ctx.m_physicalDevice, context->Surface());
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

    VkPhysicalDeviceDynamicRenderingFeaturesKHR dynamicRenderingFeature{};
    dynamicRenderingFeature.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES_KHR;
    dynamicRenderingFeature.dynamicRendering = VK_TRUE;

    VkDeviceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.pQueueCreateInfos = queueCreateInfos.data();
    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    createInfo.pEnabledFeatures = &deviceFeatures;
    createInfo.enabledExtensionCount = static_cast<uint32_t>(C_DEVICE_EXTENSIONS.size());
    createInfo.ppEnabledExtensionNames = C_DEVICE_EXTENSIONS.data();
    createInfo.pNext = &dynamicRenderingFeature;

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
    const auto indices = vulkan::FindQueueFamilies(s_ctx.m_physicalDevice, context->Surface());
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
    poolInfo.queueFamilyIndex = vulkan::FindQueueFamilies(s_ctx.m_physicalDevice, context->Surface()).graphicsFamily.value();

    RHI_ASSERT(vkCreateCommandPool(s_ctx.m_device, &poolInfo, nullptr, &m_commandPool) == VK_SUCCESS);
}

void VulkanDevice::FillProperties()
{
    VkPhysicalDeviceProperties deviceProps;
    vkGetPhysicalDeviceProperties(s_ctx.m_physicalDevice, &deviceProps);

    Parameters& properties = s_ctx.m_instance->m_parameters;

    properties.m_minUniformBufferAlignment = static_cast<uint32_t>(deviceProps.limits.minUniformBufferOffsetAlignment);
    properties.m_maxSamplerAnisotropy = deviceProps.limits.maxSamplerAnisotropy;
}

void VulkanDevice::OnResize(uint32_t x, uint32_t y)
{
    m_presentExtent = { x, y };
    m_isSwapchainDirty = true;
}

void VulkanDevice::WaitForIdle()
{
    vkDeviceWaitIdle(s_ctx.m_device);
}

}

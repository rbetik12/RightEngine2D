#include <Vulkan/Imgui/VulkanImguiProvider.hpp>
#include <Vulkan/VulkanDevice.hpp>
#include <Vulkan/VulkanRenderPass.hpp>
#include <Vulkan/VulkanHelpers.hpp>
#include <Vulkan/VulkanTexture.hpp>
#include <Vulkan/VulkanSampler.hpp>
#include <imgui.h>
#include "imgui_impl_vulkan.h"

namespace 
{

constexpr VkDescriptorPoolSize poolSizes[] =
{
        { VK_DESCRIPTOR_TYPE_SAMPLER, 100 },
        { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1024 },
        { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 100 },
        { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 100 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 100 },
        { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 100 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 100 },
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 100 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 100 },
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 100 },
        { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 100 }
};

} // unnamed

namespace rhi::vulkan::imgui
{

VulkanImguiProvider::VulkanImguiProvider() : IImguiProvider()
{
    CreateDescriptorPool();

    ImGui_ImplVulkan_InitInfo initInfo{};
    initInfo.Instance = VulkanDevice::s_ctx.m_instance->Context()->Instance();
    initInfo.PhysicalDevice = VulkanDevice::s_ctx.m_physicalDevice;
    initInfo.Device = VulkanDevice::s_ctx.m_device;
    initInfo.Queue = VulkanDevice::s_ctx.m_instance->GraphicsQueue();
    initInfo.DescriptorPool = m_descriptorPool;
    initInfo.MinImageCount = 3;
    initInfo.ImageCount = 3;
    initInfo.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
    initInfo.UseDynamicRendering = true;
    initInfo.ColorAttachmentFormat = helpers::Format(rhi::Format::BGRA8_UNORM);

    ImGui_ImplVulkan_Init(&initInfo, nullptr);
}

VulkanImguiProvider::~VulkanImguiProvider()
{
    VulkanDevice::s_ctx.m_instance->WaitForIdle();
    ImGui_ImplVulkan_Shutdown();
    vkDestroyDescriptorPool(VulkanDevice::s_ctx.m_device, m_descriptorPool, nullptr);
}

void VulkanImguiProvider::Begin()
{
    ImGui_ImplVulkan_NewFrame();
}

void VulkanImguiProvider::End()
{
    auto renderPass = std::static_pointer_cast<VulkanRenderPass>(m_renderPass);

    VkRenderingInfoKHR renderingInfo{};
    renderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO_KHR;
    renderingInfo.renderArea = helpers::Rect(m_renderPass->Descriptor().m_extent);
    renderingInfo.layerCount = 1;
    renderingInfo.colorAttachmentCount = static_cast<uint32_t>(renderPass->Descriptor().m_colorAttachments.size());
    renderingInfo.pColorAttachments = renderPass->ColorAttachments().data();

    if (renderPass->Descriptor().m_depthStencilAttachment.m_texture != nullptr) 
    {
        renderingInfo.pDepthAttachment = &renderPass->DepthAttachment();
        renderingInfo.pStencilAttachment = &renderPass->DepthAttachment();
    }

    auto cmdBuffer = VulkanDevice::s_ctx.m_instance->CurrentCmdBuffer();

    for (auto &texture : renderPass->Descriptor().m_colorAttachments) 
    {
        auto vkTexture = std::static_pointer_cast<VulkanTexture>(texture.m_texture);
        vkTexture->ChangeImageLayout(cmdBuffer, vkTexture->Layout(), VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    }

    vkCmdBeginRendering(cmdBuffer, &renderingInfo);

    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cmdBuffer);

    vkCmdEndRendering(cmdBuffer);

    for (auto &texture : renderPass->Descriptor().m_colorAttachments) 
    {
        auto vkTexture = std::static_pointer_cast<VulkanTexture>(texture.m_texture);
        vkTexture->ChangeImageLayout(cmdBuffer, vkTexture->Layout(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    }
}
void VulkanImguiProvider::Image(const std::shared_ptr<Texture>& texture, const ImVec2& size, 
        const ImVec2& uv0, const ImVec2& uv1)
{
    const auto set = GetDescriptorSet(texture);
    ImGui::Image(set, size, uv0, uv1);
}

void VulkanImguiProvider::RemoveImage(const std::shared_ptr<Texture>& texture)
{
    RHI_ASSERT(texture->GetSampler() && texture->Descriptor().m_width > 0 && texture->Descriptor().m_height > 0);

    const auto tex = std::static_pointer_cast<VulkanTexture>(texture);
    const auto texIt = m_imageViewToDescSet.find(tex->ImageView(0));

    if (texIt == m_imageViewToDescSet.end())
    {
        RHI_ASSERT(false);
        return;
    }

    m_imageViewToDescSet.erase(texIt);
}

void VulkanImguiProvider::CreateDescriptorPool()
{
    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    poolInfo.maxSets = 1000;
    poolInfo.poolSizeCount = static_cast<uint32_t>(std::size(poolSizes));
    poolInfo.pPoolSizes = poolSizes;

    RHI_ASSERT(vkCreateDescriptorPool(VulkanDevice::s_ctx.m_device, &poolInfo,
                                      nullptr,
                                      &m_descriptorPool) == VK_SUCCESS);
}

VkDescriptorSet VulkanImguiProvider::GetDescriptorSet(const std::shared_ptr<Texture>& texture)
{
    RHI_ASSERT(texture->GetSampler() && texture->Descriptor().m_width > 0 && texture->Descriptor().m_height > 0);

    const auto vkTexture = std::static_pointer_cast<VulkanTexture>(texture);
    const auto vkSampler = std::static_pointer_cast<VulkanSampler>(vkTexture->GetSampler());

    if (m_imageViewToDescSet.find(vkTexture->ImageView(0)) == m_imageViewToDescSet.end())
    {
        m_imageViewToDescSet[vkTexture->ImageView(0)] = ImGui_ImplVulkan_AddTexture(vkSampler->Raw(),
            vkTexture->ImageView(0),
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    }
    return m_imageViewToDescSet.at(vkTexture->ImageView(0));
}

} // rhi::vulkan::imgui

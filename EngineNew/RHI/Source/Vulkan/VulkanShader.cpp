#include "VulkanShader.hpp"
#include "VulkanDevice.hpp"
#include "VulkanHelpers.hpp"

#define MAX_PUSH_CONSTANT_SIZE 128

namespace rhi::vulkan
{

VulkanShader::VulkanShader(const ShaderDescriptor& descriptor) : Shader(descriptor)
{
	RHI_ASSERT(!descriptor.m_reflection.m_inputLayout.Elements().empty());
    RHI_ASSERT(!descriptor.m_blobByStage.empty());

    for (const auto& [stage, blob] : descriptor.m_blobByStage)
    {
        VkShaderModuleCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = blob.size();
        createInfo.pCode = static_cast<const uint32_t*>(blob.raw());

        VkShaderModule module = nullptr;

        if (vkCreateShaderModule(VulkanDevice::s_ctx.m_device, &createInfo, nullptr, &module) != VK_SUCCESS)
        {
            log::error("[Vulkan] Failed to create shader module for '{}' for stage: '{}'", descriptor.m_path, ShaderStageToString(stage));
            return;
        }

        m_modules[stage] = module;
    }

	log::debug("[Vulkan] Successfully created shader '{}'", descriptor.m_path);

	FillVertexData();
    CreateDescriptorSetLayout();
}

VulkanShader::~VulkanShader()
{
    for (const auto [_, module] : m_modules)
    {
        vkDestroyShaderModule(VulkanDevice::s_ctx.m_device, module, nullptr);
    }
}

void VulkanShader::CreateDescriptorSetLayout()
{
    eastl::vector<VkDescriptorSetLayoutBinding> bindings;

    for (const auto& [slot, info] : m_descriptor.m_reflection.m_bufferMap)
    {
        if (info.m_type == BufferType::UNIFORM)
        {
            VkDescriptorSetLayoutBinding bufferLayoutBinding{};
            bufferLayoutBinding.binding = slot;
            bufferLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            bufferLayoutBinding.descriptorCount = 1;
            bufferLayoutBinding.stageFlags = helpers::ShaderStage(info.m_stage);
            bindings.emplace_back(bufferLayoutBinding);
        }
    }

    for (const auto& info : m_descriptor.m_reflection.m_textures)
    {
        VkDescriptorSetLayoutBinding textureLayoutBinding{};
        textureLayoutBinding.binding = info.m_slot;
        textureLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        textureLayoutBinding.descriptorCount = 1;
        textureLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
        bindings.emplace_back(textureLayoutBinding);
    }

    if (bindings.empty())
    {
        return;
    }

    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
    layoutInfo.pBindings = bindings.data();

    RHI_ASSERT(vkCreateDescriptorSetLayout(VulkanDevice::s_ctx.m_device, &layoutInfo, nullptr, &m_layout) == VK_SUCCESS);
}

void VulkanShader::FillPushContansts()
{
    for (const auto& [slot, buffer] : Descriptor().m_reflection.m_bufferMap)
    {
        if (buffer.m_type == BufferType::CONSTANT)
        {
            RHI_ASSERT(m_constants.empty());
            VkPushConstantRange range;
            range.offset = 0;
            range.size = MAX_PUSH_CONSTANT_SIZE;
            range.stageFlags = helpers::ShaderStage(buffer.m_stage);
            m_constants.emplace_back(range);
        }
    }
}

void VulkanShader::FillVertexData()
{
    const auto& layout = m_descriptor.m_reflection.m_inputLayout;
    const auto& layoutElements = layout.Elements();
    int32_t offset = -static_cast<int32_t>(layoutElements.front().GetSize());
    for (int i = 0; i < layoutElements.size(); i++)
    {
        VkVertexInputAttributeDescription attributeDescription;
        attributeDescription.binding = 0;
        attributeDescription.location = i;
        attributeDescription.format = helpers::Format(layoutElements[i].m_type);
        attributeDescription.offset = offset + layoutElements[std::max(0, i - 1)].GetSize();
        offset += layoutElements[i].GetSize();

        m_attributesDescription.emplace_back(attributeDescription);
    }

    m_inputDescription.binding = 0;
    m_inputDescription.stride = layout.Stride();
    m_inputDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
}

} // namespace rhi::vulkan

#include "VulkanShader.hpp"
#include "Assert.hpp"
#include "Path.hpp"
#include "Logger.hpp"
#include "Device.hpp"
#include "VulkanDevice.hpp"
#include "VulkanConverters.hpp"
#include "Application.hpp"
#include <Engine/Service/Render/RenderService.hpp>
#include <vulkan/vulkan.h>
#include <fstream>

using namespace RightEngine;

namespace
{
    VkShaderModule CreateShaderModule(const core::Blob& spirv)
    {
        VkShaderModuleCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = spirv.size();
        createInfo.pCode = static_cast<const uint32_t*>(spirv.raw());

        VkShaderModule shaderModule;

        auto vulkanDevice = std::static_pointer_cast<VulkanDevice>(Device::Get());
        if (vkCreateShaderModule(vulkanDevice->GetDevice(), &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
        {
            R_CORE_ASSERT(false, "failed to create shader module!");
        }

        return shaderModule;
    }
}

VulkanShader::VulkanShader(const std::shared_ptr<Device>& device, const ShaderProgramDescriptor& descriptor) : Shader(device, descriptor)
{
    R_CORE_ASSERT(descriptor.layout.GetElements().size() > 0, "");
    ShaderDescriptor vertexShader;
    ShaderDescriptor fragmentShader;

    for (const auto& desc : descriptor.shaders)
    {
        switch (desc.type)
        {
            case ShaderType::VERTEX:
                vertexShader = desc;
                break;
            case ShaderType::FRAGMENT:
                fragmentShader = desc;
                break;
            default:
                R_CORE_ASSERT(false, "");
        }
    }

    const auto vertexShaderPath = Path::Absolute(vertexShader.path);
    const auto fragmentShaderPath = Path::Absolute(fragmentShader.path);

    auto& rs = Instance().Service<engine::render::RenderService>();
    auto vertexShaderData = rs.CompileShader(vertexShaderPath);
    auto fragmentShaderData = rs.CompileShader(fragmentShaderPath);
    vertexShaderModule = CreateShaderModule(vertexShaderData.get().m_compiledShader);
    fragShaderModule = CreateShaderModule(fragmentShaderData.get().m_compiledShader);
}

VulkanShader::~VulkanShader()
{
    auto device = VK_DEVICE();
    vkDestroyShaderModule(device->GetDevice(), fragShaderModule, nullptr);
    vkDestroyShaderModule(device->GetDevice(), vertexShaderModule, nullptr);
}

void VulkanShader::Bind() const
{

}

void VulkanShader::UnBind() const
{

}

VkVertexInputBindingDescription VulkanShader::GetVertexFormatDescription() const
{
    VkVertexInputBindingDescription bindingDescription{};
    bindingDescription.binding = 0;
    bindingDescription.stride = descriptor.layout.GetStride();
    bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    return bindingDescription;
}

std::vector<VkVertexInputAttributeDescription> VulkanShader::GetVertexFormatAttribute() const
{
    std::vector<VkVertexInputAttributeDescription> attributeDescriptions;

    const auto layoutElements = descriptor.layout.GetElements();
    uint32_t offset = -layoutElements.front().GetSize();
    for (int i = 0; i < layoutElements.size(); i++)
    {
        VkVertexInputAttributeDescription attributeDescription;
        attributeDescription.binding = 0;
        attributeDescription.location = i;
        attributeDescription.format = VulkanConverters::Format(layoutElements[i].type);
        attributeDescription.offset = offset + layoutElements[std::max(0, i - 1)].GetSize();
        offset += layoutElements[i].GetSize();;

        attributeDescriptions.emplace_back(attributeDescription);
    }

    return attributeDescriptions;
}

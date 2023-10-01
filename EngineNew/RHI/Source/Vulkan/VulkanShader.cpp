#include "VulkanShader.hpp"
#include "VulkanDevice.hpp"
#include "VulkanHelpers.hpp"

namespace rhi::vulkan
{
	VulkanShader::VulkanShader(const ShaderDescriptor& descriptor) : Shader(descriptor)
	{
		RHI_ASSERT(!descriptor.m_reflection.m_inputLayout.Elements().empty());

        VkShaderModuleCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = descriptor.m_blob.size();
        createInfo.pCode = static_cast<const uint32_t*>(descriptor.m_blob.raw());

        if (vkCreateShaderModule(VulkanDevice::s_ctx.m_device, &createInfo, nullptr, &m_module) != VK_SUCCESS)
        {
            log::error("[Vulkan] Failed to create shader '{}'", descriptor.m_path);
            return;
        }

		log::debug("[Vulkan] Successfully created shader '{}'", descriptor.m_path);

        if (descriptor.m_type == ShaderType::VERTEX)
        {
            fillVertexData();
        }
	}

	VulkanShader::~VulkanShader()
	{
        vkDestroyShaderModule(VulkanDevice::s_ctx.m_device, m_module, nullptr);
	}

	void VulkanShader::fillVertexData()
	{
        RHI_ASSERT(m_descriptor.m_type == ShaderType::VERTEX);

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

            m_vertexBufferAttributesDescription.emplace_back(attributeDescription);
        }

        m_vertexBufferDescription.binding = 0;
        m_vertexBufferDescription.stride = layout.Stride();
        m_vertexBufferDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
	}
}

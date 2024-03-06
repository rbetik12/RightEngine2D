#include "VulkanPipeline.hpp"
#include "VulkanShader.hpp"
#include "VulkanHelpers.hpp"
#include "VulkanDevice.hpp"
#include "VulkanRenderPass.hpp"
#include <RHI/RenderPass.hpp>
#include <vulkan/vulkan.h>

namespace rhi::vulkan
{

VulkanPipeline::VulkanPipeline(const PipelineDescriptor& descriptor) : Pipeline(descriptor)
{
    RHI_ASSERT(descriptor.m_shader);

    const auto vkShader = std::static_pointer_cast<VulkanShader>(descriptor.m_shader);

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;

    VkDescriptorSetLayout layout = vkShader->Layout();

    if (layout)
    {
        pipelineLayoutInfo.setLayoutCount = 1;
        pipelineLayoutInfo.pSetLayouts = &layout;
    }
    else
    {
        pipelineLayoutInfo.setLayoutCount = 0;
        pipelineLayoutInfo.pSetLayouts = nullptr;
    }

    const auto pushConstants = vkShader->Constants();

    if (pushConstants.empty())
    {
        pipelineLayoutInfo.pushConstantRangeCount = 0;
        pipelineLayoutInfo.pPushConstantRanges = nullptr;
    }
    else
    {
        pipelineLayoutInfo.pushConstantRangeCount = static_cast<uint32_t>(pushConstants.size());
        pipelineLayoutInfo.pPushConstantRanges = pushConstants.data();
    }

    RHI_ASSERT(vkCreatePipelineLayout(VulkanDevice::s_ctx.m_device, &pipelineLayoutInfo, nullptr, &m_layout) == VK_SUCCESS);

    if (descriptor.m_compute)
    {
        CreateComputePipeline();
    }
    else
    {
        CreateFxPipeline();
    }
}

VulkanPipeline::~VulkanPipeline()
{
    vkDestroyPipelineLayout(VulkanDevice::s_ctx.m_device, m_layout, nullptr);
    vkDestroyPipeline(VulkanDevice::s_ctx.m_device, m_pipeline, nullptr);
}

void VulkanPipeline::CreateFxPipeline()
{
    RHI_ASSERT(m_descriptor.m_pass);

    const eastl::vector<VkDynamicState> dynamicStates =
    {
            VK_DYNAMIC_STATE_VIEWPORT,
            VK_DYNAMIC_STATE_SCISSOR
    };

    VkPipelineDynamicStateCreateInfo dynamicState{};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
    dynamicState.pDynamicStates = dynamicStates.data();

    std::shared_ptr<VulkanShader> shader = std::static_pointer_cast<VulkanShader>(m_descriptor.m_shader);

    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = 1;
    VkVertexInputBindingDescription bindingDescription[] = { shader->InputDescription() };
    vertexInputInfo.pVertexBindingDescriptions = bindingDescription;
    const auto vertexFormatAttribute = shader->AttributeDescription();
    vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(vertexFormatAttribute.size());
    vertexInputInfo.pVertexAttributeDescriptions = vertexFormatAttribute.data();

    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    const auto renderPassDescriptor = m_descriptor.m_pass->Descriptor();

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(renderPassDescriptor.m_extent.x);
    viewport.height = static_cast<float>(renderPassDescriptor.m_extent.y);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor{};
    scissor.offset = { 0, 0 };
    scissor.extent = helpers::Extent(renderPassDescriptor.m_extent);

    VkPipelineViewportStateCreateInfo viewportState{};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.pViewports = &viewport;
    viewportState.scissorCount = 1;
    viewportState.pScissors = &scissor;

    VkPipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = helpers::CullMode(m_descriptor.m_cullMode);
    rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;

    VkPipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisampling.minSampleShading = 1.0f; // Optional
    multisampling.pSampleMask = nullptr; // Optional
    multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
    multisampling.alphaToOneEnable = VK_FALSE; // Optional

    eastl::vector<VkPipelineColorBlendAttachmentState> colorBlendAttachments;
    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_FALSE;
    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
    colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
    colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD; // Optional
    colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
    colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
    colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD; // Optional

    for (int i = 0; i < renderPassDescriptor.m_colorAttachments.size(); i++)
    {
        colorBlendAttachments.push_back(colorBlendAttachment);
    }

    VkPipelineColorBlendStateCreateInfo colorBlending{};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
    colorBlending.attachmentCount = static_cast<uint32_t>(colorBlendAttachments.size());
    colorBlending.pAttachments = colorBlendAttachments.data();
    colorBlending.blendConstants[0] = 0.0f; // Optional
    colorBlending.blendConstants[1] = 0.0f; // Optional
    colorBlending.blendConstants[2] = 0.0f; // Optional
    colorBlending.blendConstants[3] = 0.0f; // Optional

    eastl::vector<VkPipelineShaderStageCreateInfo> shaderStages;

    for (const auto& [stage, blob] : shader->Descriptor().m_blobByStage)
    {
        VkPipelineShaderStageCreateInfo shaderStageInfo{};
        shaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shaderStageInfo.stage = helpers::ShaderStage(stage);
        shaderStageInfo.module = shader->Module(stage);
        shaderStageInfo.pName = "main";

        shaderStages.emplace_back(shaderStageInfo);
    }

    RHI_ASSERT(!shaderStages.empty());

    VkPipelineDepthStencilStateCreateInfo depthStencil{};
    depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencil.depthTestEnable = VK_TRUE;
    depthStencil.depthWriteEnable = VK_TRUE;
    depthStencil.depthCompareOp = helpers::CompareOp(m_descriptor.m_depthCompareOp);
    depthStencil.depthBoundsTestEnable = VK_FALSE;
    depthStencil.minDepthBounds = 0.0f; // Optional
    depthStencil.maxDepthBounds = 1.0f; // Optional
    depthStencil.stencilTestEnable = VK_FALSE;
    depthStencil.front = {}; // Optional
    depthStencil.back = {}; // Optional

    eastl::vector<VkFormat> colorAttachmentFormats;

    for (auto& attachment : renderPassDescriptor.m_colorAttachments)
    {
        colorAttachmentFormats.emplace_back(helpers::Format(attachment.m_texture->Descriptor().m_format));
    }

    VkPipelineRenderingCreateInfoKHR pipelineRenderingInfo{};
    pipelineRenderingInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO_KHR;
    pipelineRenderingInfo.colorAttachmentCount = static_cast<uint32_t>(renderPassDescriptor.m_colorAttachments.size());
    pipelineRenderingInfo.pColorAttachmentFormats = colorAttachmentFormats.data();

    if (renderPassDescriptor.m_depthStencilAttachment.m_texture)
    {
        pipelineRenderingInfo.depthAttachmentFormat = helpers::Format(renderPassDescriptor.m_depthStencilAttachment.m_texture->Descriptor().m_format);
        pipelineRenderingInfo.stencilAttachmentFormat = helpers::Format(renderPassDescriptor.m_depthStencilAttachment.m_texture->Descriptor().m_format);
    }

    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.pNext = &pipelineRenderingInfo;
    pipelineInfo.stageCount = static_cast<uint32_t>(shaderStages.size());
    pipelineInfo.pStages = shaderStages.data();
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pDepthStencilState = &depthStencil;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDynamicState = &dynamicState;
    pipelineInfo.layout = m_layout;
    pipelineInfo.renderPass = nullptr;
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = nullptr; // Optional
    pipelineInfo.basePipelineIndex = -1; // Optional

    RHI_ASSERT(vkCreateGraphicsPipelines(VulkanDevice::s_ctx.m_device, nullptr, 1, &pipelineInfo, nullptr, &m_pipeline) == VK_SUCCESS);
}

void VulkanPipeline::CreateComputePipeline()
{
    VkPipelineShaderStageCreateInfo shaderStageInfo{};
    shaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStageInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
    shaderStageInfo.module = std::static_pointer_cast<VulkanShader>(m_descriptor.m_shader)->Module(ShaderStage::COMPUTE);
    shaderStageInfo.pName = "main";

    VkComputePipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
    pipelineInfo.layout = m_layout;
    pipelineInfo.stage = shaderStageInfo;

    RHI_ASSERT(vkCreateComputePipelines(VulkanDevice::s_ctx.m_device, nullptr, 1, &pipelineInfo, nullptr, &m_pipeline) == VK_SUCCESS);
}

} // rhi::vulkan

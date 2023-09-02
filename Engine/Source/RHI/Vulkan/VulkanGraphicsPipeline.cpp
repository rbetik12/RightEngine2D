#include "VulkanGraphicsPipeline.hpp"
#include "VulkanConverters.hpp"
#include "VulkanShader.hpp"
#include "Assert.hpp"
#include "VulkanBuffer.hpp"
#include "VulkanConfig.hpp"
#include "VulkanTexture.hpp"
#include <vector>

using namespace RightEngine;

namespace
{
    std::vector<VkWriteDescriptorSet> GetWriteDescriptorSets(const std::vector<VkDescriptorSet>& descriptorSets,
                                                             const std::unordered_map<int, std::shared_ptr<Buffer>>& buffers)
    {
        static std::vector<VkDescriptorBufferInfo> bufferInfos;
        std::vector<VkWriteDescriptorSet> writeDescriptorSet;
        for (const auto& [slot, buffer] : buffers)
        {
            if (buffer->GetDescriptor().type == BufferType::CONSTANT)
            {
                continue;
            }

            VkDescriptorBufferInfo bufferInfo{};
            bufferInfo.buffer = std::static_pointer_cast<VulkanBuffer>(buffer)->GetBuffer();
            bufferInfo.offset = 0;
            bufferInfo.range = buffer->GetDescriptor().size;

            bufferInfos.push_back(bufferInfo);

            VkWriteDescriptorSet descriptorWrite{};
            descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrite.dstSet = descriptorSets[0];
            descriptorWrite.dstBinding = slot;
            descriptorWrite.dstArrayElement = 0;
            descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            descriptorWrite.descriptorCount = 1;
            descriptorWrite.pBufferInfo = &bufferInfos.back();

            writeDescriptorSet.push_back(descriptorWrite);
        }

        return writeDescriptorSet;
    }

    void ResizeAttachment(AttachmentDescriptor& attachment, int x, int y)
    {
        TextureDescriptor descriptor{};
        const auto sampler = attachment.texture->GetSampler();
        descriptor = attachment.texture->GetSpecification();
        descriptor.width = x;
        descriptor.height = y;
        attachment.texture = Device::Get()->CreateTexture(descriptor, {});
        if (sampler)
        {
            attachment.texture->SetSampler(sampler);
        }
    }
}

VulkanGraphicsPipeline::VulkanGraphicsPipeline(const GraphicsPipelineDescriptor& descriptor,
                                               const RenderPassDescriptor& renderPassDescriptor) : GraphicsPipeline(descriptor, renderPassDescriptor)
{
    Init(descriptor, renderPassDescriptor);
    for (const auto& attachment : renderPassDescriptor.colorAttachments)
    {
        glm::vec4 color = attachment.clearValue.color;
        VkClearValue clearValue;
        clearValue.color = { color.r, color.g, color.b, color.a };
        m_clearValues.push_back(clearValue);
    }
    VkClearValue clearValue;
    clearValue.depthStencil = { renderPassDescriptor.depthStencilAttachment.clearValue.depth,
                                renderPassDescriptor.depthStencilAttachment.clearValue.stencil };
    m_clearValues.push_back(clearValue);
}

void VulkanGraphicsPipeline::Init(const GraphicsPipelineDescriptor& descriptor,
                                  const RenderPassDescriptor& renderPassDescriptor)
{
    if (!descriptor.shader)
    {
        CreateRenderPass(renderPassDescriptor);
        CreateFramebuffer();
        return;
    }
    std::vector<VkDynamicState> dynamicStates =
            {
                    VK_DYNAMIC_STATE_VIEWPORT,
                    VK_DYNAMIC_STATE_SCISSOR
            };

    VkPipelineDynamicStateCreateInfo dynamicState{};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
    dynamicState.pDynamicStates = dynamicStates.data();

    const auto shader = std::static_pointer_cast<VulkanShader>(descriptor.shader);
    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = 1;
    VkVertexInputBindingDescription bindingDescription[] = { shader->GetVertexFormatDescription() };
    vertexInputInfo.pVertexBindingDescriptions = bindingDescription;
    const auto vertexFormatAttribute = shader->GetVertexFormatAttribute();
    vertexInputInfo.vertexAttributeDescriptionCount = vertexFormatAttribute.size();
    vertexInputInfo.pVertexAttributeDescriptions = vertexFormatAttribute.data();

    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float) renderPassDescriptor.extent.x;
    viewport.height = (float) renderPassDescriptor.extent.y;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = VulkanConverters::Extent(renderPassDescriptor.extent);

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
    rasterizer.cullMode = VulkanConverters::CullMode(pipelineDescriptor.cullMode);
    rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;
    rasterizer.depthBiasConstantFactor = 0.0f; // Optional
    rasterizer.depthBiasClamp = 0.0f; // Optional
    rasterizer.depthBiasSlopeFactor = 0.0f; // Optional

    VkPipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisampling.minSampleShading = 1.0f; // Optional
    multisampling.pSampleMask = nullptr; // Optional
    multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
    multisampling.alphaToOneEnable = VK_FALSE; // Optional

    std::vector<VkPipelineColorBlendAttachmentState> colorBlendAttachments;
    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_FALSE;
    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
    colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
    colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD; // Optional
    colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
    colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
    colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD; // Optional

    for (int i = 0; i < renderPassDescriptor.colorAttachments.size(); i++)
    {
        colorBlendAttachments.push_back(colorBlendAttachment);
    }

    VkPipelineColorBlendStateCreateInfo colorBlending{};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
    colorBlending.attachmentCount = colorBlendAttachments.size();
    colorBlending.pAttachments = colorBlendAttachments.data();
    colorBlending.blendConstants[0] = 0.0f; // Optional
    colorBlending.blendConstants[1] = 0.0f; // Optional
    colorBlending.blendConstants[2] = 0.0f; // Optional
    colorBlending.blendConstants[3] = 0.0f; // Optional

    VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
    vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertShaderStageInfo.module = std::static_pointer_cast<VulkanShader>(descriptor.shader)->GetShaderModule(ShaderModuleType::VERTEX);
    vertShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
    fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageInfo.module = std::static_pointer_cast<VulkanShader>(descriptor.shader)->GetShaderModule(ShaderModuleType::FRAGMENT);
    fragShaderStageInfo.pName = "main";

    CreateDescriptorSets();

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    if (descriptorSetLayout)
    {
        pipelineLayoutInfo.setLayoutCount = 1;
        pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;
    }
    else
    {
        pipelineLayoutInfo.setLayoutCount = 0;
        pipelineLayoutInfo.pSetLayouts = VK_NULL_HANDLE;
    }

    for (const auto& [ref, type] : shader->GetShaderProgramDescriptor().reflection.buffers)
    {
        if (type == BufferType::CONSTANT)
        {
            R_CORE_ASSERT(pushConstants.empty(), "");
            VkPushConstantRange range;
            range.offset = 0;
            range.size = MAX_PUSH_CONSTANT_SIZE;
            range.stageFlags = VulkanConverters::ShaderStage(ref.stage);
            pushConstants.emplace_back(range);
        }
    }

    if (pushConstants.empty())
    {
        pipelineLayoutInfo.pushConstantRangeCount = 0;
        pipelineLayoutInfo.pPushConstantRanges = nullptr;
    }
    else
    {
        pipelineLayoutInfo.pushConstantRangeCount = pushConstants.size();
        pipelineLayoutInfo.pPushConstantRanges = pushConstants.data();
    }

    std::vector<VkPipelineShaderStageCreateInfo> shaderStages{ vertShaderStageInfo, fragShaderStageInfo };

    VkPipelineDepthStencilStateCreateInfo depthStencil{};
    depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencil.depthTestEnable = VK_TRUE;
    depthStencil.depthWriteEnable = VK_TRUE;
    depthStencil.depthCompareOp = VulkanConverters::CompareOp(pipelineDescriptor.depthCompareOp);
    depthStencil.depthBoundsTestEnable = VK_FALSE;
    depthStencil.minDepthBounds = 0.0f; // Optional
    depthStencil.maxDepthBounds = 1.0f; // Optional
    depthStencil.stencilTestEnable = VK_FALSE;
    depthStencil.front = {}; // Optional
    depthStencil.back = {}; // Optional

    if (vkCreatePipelineLayout(VK_DEVICE()->GetDevice(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
    {
        R_CORE_ASSERT(false, "failed to create pipeline layout!");
    }

    CreateRenderPass(renderPassDescriptor);

    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = shaderStages.size();
    pipelineInfo.pStages = shaderStages.data();
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pDepthStencilState = &depthStencil;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDynamicState = &dynamicState;
    pipelineInfo.layout = pipelineLayout;
    pipelineInfo.renderPass = renderPass;
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
    pipelineInfo.basePipelineIndex = -1; // Optional

    if (vkCreateGraphicsPipelines(VK_DEVICE()->GetDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline) != VK_SUCCESS)
    {
        R_CORE_ASSERT(false, "failed to create graphics pipeline!");
    }

    CreateFramebuffer();
}

void VulkanGraphicsPipeline::CreateRenderPass(const RenderPassDescriptor& renderPassDescriptor)
{
    std::vector<VkAttachmentDescription> colorAttachments;
    for (const auto& attachment : renderPassDescriptor.colorAttachments)
    {
        VkAttachmentDescription colorAttachment{};
        colorAttachment.format = VulkanConverters::Format(attachment.texture->GetSpecification().format);
        colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        colorAttachment.loadOp = VulkanConverters::LoadOperation(attachment.loadOperation);
        colorAttachment.storeOp = VulkanConverters::StoreOperation(attachment.storeOperation);
        colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        if (attachment.loadOperation == AttachmentLoadOperation::LOAD)
        {
            colorAttachment.initialLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        }
        colorAttachment.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        colorAttachments.push_back(colorAttachment);
    }

    VkAttachmentDescription depthAttachment{};
    if (renderPassDescriptor.depthStencilAttachment.texture)
    {
        depthAttachment.format = VulkanConverters::Format(renderPassDescriptor.depthStencilAttachment.texture->GetSpecification().format);
    }
    else
    {
        depthAttachment.format = VK_FORMAT_D32_SFLOAT_S8_UINT;
    }
    depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    depthAttachment.loadOp = VulkanConverters::LoadOperation(renderPassDescriptor.depthStencilAttachment.loadOperation);
    depthAttachment.storeOp = VulkanConverters::StoreOperation(renderPassDescriptor.depthStencilAttachment.storeOperation);
    depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    if (depthAttachment.loadOp == VK_ATTACHMENT_LOAD_OP_LOAD)
    {
        depthAttachment.initialLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    }
    else
    {
        depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    }
    depthAttachment.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    std::vector<VkAttachmentReference> colorAttachmentRefs;

    for (int i = 0; i < colorAttachments.size(); i++)
    {
        VkAttachmentReference colorAttachmentRef{};
        colorAttachmentRef.attachment = i;
        colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        colorAttachmentRefs.push_back(colorAttachmentRef);
    }

    VkAttachmentReference depthAttachmentRef{};
    depthAttachmentRef.attachment = colorAttachmentRefs.size();
    depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = colorAttachmentRefs.size();
    subpass.pColorAttachments = colorAttachmentRefs.data();
    const bool hasDepthAttachment = renderPassDescriptor.depthStencilAttachment.texture != nullptr;
    if (hasDepthAttachment)
    {
        subpass.pDepthStencilAttachment = &depthAttachmentRef;
    }

    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcAccessMask = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    if (hasDepthAttachment)
    {
        colorAttachments.push_back(depthAttachment);
    }
    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = colorAttachments.size();
    renderPassInfo.pAttachments = colorAttachments.data();
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    if (vkCreateRenderPass(VK_DEVICE()->GetDevice(), &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS)
    {
        R_CORE_ASSERT(false, "failed to create render pass!");
    }
}

void VulkanGraphicsPipeline::CreateDescriptorSets()
{
    std::vector<VkDescriptorSetLayoutBinding> bindings;
    for (const auto& [bufferRef, bufferType] : pipelineDescriptor.shader->GetShaderProgramDescriptor().reflection.buffers)
    {
        if (bufferType == BufferType::UNIFORM)
        {
            VkDescriptorSetLayoutBinding bufferLayoutBinding{};
            bufferLayoutBinding.binding = bufferRef.slot;
            bufferLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            bufferLayoutBinding.descriptorCount = 1;
            bufferLayoutBinding.stageFlags = bufferRef.stage == ShaderType::VERTEX ? VK_SHADER_STAGE_VERTEX_BIT : VK_SHADER_STAGE_FRAGMENT_BIT;
            bufferLayoutBinding.pImmutableSamplers = nullptr; // Optional
            bindings.emplace_back(bufferLayoutBinding);
        }
    }

    for (const auto& slot : pipelineDescriptor.shader->GetShaderProgramDescriptor().reflection.textures)
    {
        VkDescriptorSetLayoutBinding textureLayoutBinding{};
        textureLayoutBinding.binding = slot;
        textureLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        textureLayoutBinding.descriptorCount = 1;
        textureLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
        textureLayoutBinding.pImmutableSamplers = nullptr; // Optional
        bindings.emplace_back(textureLayoutBinding);
    }

    if (bindings.empty())
    {
        return;
    }

    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = bindings.size();
    layoutInfo.pBindings = bindings.data();

    if (vkCreateDescriptorSetLayout(VK_DEVICE()->GetDevice(), &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create descriptor set layout!");
    }
}

VulkanGraphicsPipeline::~VulkanGraphicsPipeline()
{
    vkDestroyFramebuffer(VK_DEVICE()->GetDevice(), framebuffer, nullptr);
    vkDestroyPipeline(VK_DEVICE()->GetDevice(), graphicsPipeline, nullptr);
    vkDestroyPipelineLayout(VK_DEVICE()->GetDevice(), pipelineLayout, nullptr);
    vkDestroyRenderPass(VK_DEVICE()->GetDevice(), renderPass, nullptr);

    if (descriptorSetLayout)
    {
        vkDestroyDescriptorSetLayout(VK_DEVICE()->GetDevice(), descriptorSetLayout, nullptr);
    }
}

void VulkanGraphicsPipeline::CreateFramebuffer()
{
    std::vector<VkImageView> attachments;

    for (const auto& attachment : renderPassDescriptor.colorAttachments)
    {
        TextureDescriptor texDescriptor = attachment.texture->GetSpecification();
        R_CORE_ASSERT(texDescriptor.width == renderPassDescriptor.extent.x
                      && texDescriptor.height == renderPassDescriptor.extent.y, "");
        const auto vkTexture = std::static_pointer_cast<VulkanTexture>(attachment.texture);
        attachments.push_back(vkTexture->GetImageView());
    }

    if (renderPassDescriptor.depthStencilAttachment.texture)
    {
        attachments.push_back(std::static_pointer_cast<VulkanTexture>(renderPassDescriptor.depthStencilAttachment.texture)->GetImageView());
    }

    VkFramebufferCreateInfo framebufferInfo{};
    framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferInfo.renderPass = renderPass;
    framebufferInfo.attachmentCount = attachments.size();
    framebufferInfo.pAttachments = attachments.data();
    framebufferInfo.width = renderPassDescriptor.extent.x;
    framebufferInfo.height = renderPassDescriptor.extent.y;
    framebufferInfo.layers = 1;

    if (vkCreateFramebuffer(VK_DEVICE()->GetDevice(), &framebufferInfo, nullptr, &framebuffer) != VK_SUCCESS)
    {
        R_CORE_ASSERT(false, "failed to create framebuffer!");
    }

}

void VulkanGraphicsPipeline::Resize(int x, int y)
{
    if (x == renderPassDescriptor.extent.x && renderPassDescriptor.extent.y == y)
    {
        R_CORE_WARN("Trying to resize renderpass {0} to same size!", renderPassDescriptor.name);
        return;
    }
    
    renderPassDescriptor.extent.x = x;
    renderPassDescriptor.extent.y = y;
    
    for (auto& attachment : renderPassDescriptor.colorAttachments)
    {
        ResizeAttachment(attachment, x, y);
    }

    if (renderPassDescriptor.depthStencilAttachment.texture)
    {
        ResizeAttachment(renderPassDescriptor.depthStencilAttachment, x, y);
    }

    vkDeviceWaitIdle(VK_DEVICE()->GetDevice());
    vkDestroyFramebuffer(VK_DEVICE()->GetDevice(), framebuffer, nullptr);
    CreateFramebuffer();
}

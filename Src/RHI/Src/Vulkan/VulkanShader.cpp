#include "VulkanShader.hpp"
#include "VulkanBuffer.hpp"
#include "VulkanDevice.hpp"
#include "VulkanTexture.hpp"
#include "VulkanHelpers.hpp"
#include "VulkanSampler.hpp"

#define MAX_PUSH_CONSTANT_SIZE 128

namespace rhi::vulkan
{

VulkanShader::VulkanShader(const ShaderDescriptor& descriptor) : Shader(descriptor)
{
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

    if (m_descriptor.m_type == ShaderType::FX)
    {
        FillVertexData();
    }
    CreateDescriptorSetLayout();
    AllocateDescriptorSet();
}

VulkanShader::~VulkanShader()
{
    vkDestroyDescriptorPool(VulkanDevice::s_ctx.m_device, m_descriptorPool, nullptr);
    vkDestroyDescriptorSetLayout(VulkanDevice::s_ctx.m_device, m_layout, nullptr);

    for (const auto [_, module] : m_modules)
    {
        vkDestroyShaderModule(VulkanDevice::s_ctx.m_device, module, nullptr);
    }
}

// TODO: Add validation for texture slots from reflection
void VulkanShader::SetTexture(const std::shared_ptr<Texture>& texture, uint8_t slot)
{
    m_dirty = true;

    TextureInfo info;
    info.m_texture = texture;
    info.m_slot = slot;

    m_texturesToSync.emplace_back(info);
}

// TODO: Add validation for buffer slots from reflection
void VulkanShader::SetBuffer(const std::shared_ptr<Buffer>& buffer, uint8_t slot, ShaderStage stage, int offset)
{
    m_dirty = true;

    BufferInfo info;
    info.m_buffer = buffer;
    info.m_slot = slot;
    info.m_offset = offset;
    info.m_stage = stage;

    m_buffersToSync.emplace_back(info);
}

void VulkanShader::Sync()
{
    if (!m_dirty)
    {
        return;
    }
    m_dirty = false;

    eastl::vector<VkWriteDescriptorSet> writeDescriptorSets;
    eastl::vector<VkDescriptorBufferInfo> bufferInfos;

    for (auto& buffer : m_buffersToSync)
    {
        if (buffer.m_buffer.expired())
        {
            rhi::log::warning("Expired buffer ptr in slot {} for shader '{}'", buffer.m_slot, m_descriptor.m_name);
            continue;
        }

        const auto bufferPtr = buffer.m_buffer.lock();

        VkDescriptorBufferInfo bufferInfo{};
        bufferInfo.buffer = std::static_pointer_cast<VulkanBuffer>(bufferPtr)->Raw();
        // TODO: Add validation for stride and offset
        bufferInfo.range = bufferPtr->Descriptor().m_size;
        bufferInfo.offset = buffer.m_offset;

        bufferInfos.emplace_back(bufferInfo);

        VkWriteDescriptorSet descriptorWrite{};
        descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite.dstSet = m_descriptorSet;
        descriptorWrite.dstBinding = buffer.m_slot;
        descriptorWrite.dstArrayElement = 0;
        descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrite.descriptorCount = 1;
        descriptorWrite.pBufferInfo = &bufferInfos.back();

        writeDescriptorSets.emplace_back(descriptorWrite);
    }

    eastl::vector<VkDescriptorImageInfo> textureInfos;
    // To prevent reallocation
    textureInfos.set_capacity(m_texturesToSync.size());

    for (auto& texture : m_texturesToSync)
    {
        if (texture.m_texture.expired())
        {
            rhi::log::warning("Texture in slot {} in shader '{}' is expired", texture.m_slot, m_descriptor.m_name);
            continue;
        }

        const auto texPtr = std::static_pointer_cast<VulkanTexture>(texture.m_texture.lock());

        VkDescriptorImageInfo imageInfo{};
        if (texPtr->Descriptor().m_format == Format::D32_SFLOAT_S8_UINT ||
            m_descriptor.m_reflection.m_storageImages.find({ texture.m_slot }) 
                != m_descriptor.m_reflection.m_storageImages.end())
        {
            imageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
        }
        else
        {
            imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        }

        // TODO: Here we probably need to add some index somewhere to represent which part of the texture we want to get
        imageInfo.imageView = texPtr->ImageView(0);

        // TODO: We need to see whether the texture we are currently using was changed (e.g. sampler changed) and update it here
        imageInfo.sampler = std::static_pointer_cast<VulkanSampler>(texPtr->GetSampler())->Raw();

        textureInfos.push_back(imageInfo);

        VkWriteDescriptorSet descriptorWrite{};
        descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite.dstSet = m_descriptorSet;
        descriptorWrite.dstBinding = texture.m_slot;
        descriptorWrite.dstArrayElement = 0;
        descriptorWrite.descriptorType = DescriptorType(texture.m_slot);
        descriptorWrite.descriptorCount = 1;
        descriptorWrite.pImageInfo = &textureInfos[textureInfos.size() - 1];

        writeDescriptorSets.push_back(descriptorWrite);
    }

    vkUpdateDescriptorSets(VulkanDevice::s_ctx.m_device, 
        static_cast<uint32_t>(writeDescriptorSets.size()), 
        writeDescriptorSets.data(), 
        0,
        nullptr);

    m_texturesToSync.clear();
    m_buffersToSync.clear();
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
        textureLayoutBinding.stageFlags = m_descriptor.m_type == ShaderType::FX ? VK_SHADER_STAGE_FRAGMENT_BIT : VK_SHADER_STAGE_COMPUTE_BIT;
        bindings.emplace_back(textureLayoutBinding);
    }

    for (const auto& info : m_descriptor.m_reflection.m_storageImages)
    {
        RHI_ASSERT(m_descriptor.m_type == ShaderType::COMPUTE);

        VkDescriptorSetLayoutBinding textureLayoutBinding{};
        textureLayoutBinding.binding = info.m_slot;
        textureLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
        textureLayoutBinding.descriptorCount = 1;
        textureLayoutBinding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
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
    m_inputDescription.binding = 0;
    m_inputDescription.stride = layout.Stride();
    m_inputDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    if (layout.Empty())
    {
        return;
    }

    const auto& layoutElements = layout.Elements();

    // Here we use negative offset, so we can correctly calculate offset of the first element - 0 bytes
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
}

void VulkanShader::AllocateDescriptorSet()
{
    VkDescriptorPoolSize bufferPoolSize{};
    bufferPoolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    bufferPoolSize.descriptorCount = static_cast<uint32_t>(m_descriptor.m_reflection.m_bufferMap.size());

    VkDescriptorPoolSize texturePoolSize{};
    texturePoolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    texturePoolSize.descriptorCount = static_cast<uint32_t>(m_descriptor.m_reflection.m_textures.size());

    VkDescriptorPoolSize imageStoragePoolSize{};
    imageStoragePoolSize.type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    imageStoragePoolSize.descriptorCount = static_cast<uint32_t>(m_descriptor.m_reflection.m_storageImages.size());

    VkDescriptorPoolSize storageBufferPoolSize{};
    storageBufferPoolSize.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    storageBufferPoolSize.descriptorCount = static_cast<uint32_t>(m_descriptor.m_reflection.m_storageBufferMap.size());

    if (bufferPoolSize.descriptorCount == 0 
        && texturePoolSize.descriptorCount == 0
        && imageStoragePoolSize.descriptorCount == 0
        && storageBufferPoolSize.descriptorCount == 0)
    {
        return;
    }

    eastl::vector<VkDescriptorPoolSize> poolSizes;
    if (bufferPoolSize.descriptorCount > 0)
    {
        poolSizes.push_back(bufferPoolSize);
    }
    else if (texturePoolSize.descriptorCount > 0)
    {
        poolSizes.push_back(texturePoolSize);
    }
    else if (imageStoragePoolSize.descriptorCount > 0)
    {
        poolSizes.push_back(imageStoragePoolSize);
    }
    else if (storageBufferPoolSize.descriptorCount > 0)
    {
        poolSizes.push_back(storageBufferPoolSize);
    }

    RHI_ASSERT(!poolSizes.empty());

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    poolInfo.pPoolSizes = poolSizes.data();
    poolInfo.maxSets = 1;

    RHI_ASSERT(vkCreateDescriptorPool(VulkanDevice::s_ctx.m_device, &poolInfo, nullptr, &m_descriptorPool) == VK_SUCCESS);

    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = m_descriptorPool;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &m_layout;

    RHI_ASSERT(vkAllocateDescriptorSets(VulkanDevice::s_ctx.m_device, &allocInfo, &m_descriptorSet) == VK_SUCCESS);
}

VkDescriptorType VulkanShader::DescriptorType(uint8_t slot)
{
    const auto texIt = m_descriptor.m_reflection.m_textures.find({ slot });
    const auto storageImgIt = m_descriptor.m_reflection.m_storageImages.find({ slot });

    if (texIt != m_descriptor.m_reflection.m_textures.end())
    {
        return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    }

    if (storageImgIt != m_descriptor.m_reflection.m_storageImages.end())
    {
        return VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    }

    RHI_ASSERT(false);
    return VK_DESCRIPTOR_TYPE_MAX_ENUM;
}

} // namespace rhi::vulkan

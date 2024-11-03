#include "VulkanGPUMaterial.hpp"
#include "VulkanBuffer.hpp"
#include "VulkanSampler.hpp"
#include "VulkanTexture.hpp"
#include "VulkanDevice.hpp"

namespace
{
constexpr uint32_t C_DESCRIPTOR_POOL_SIZE = 128;
}

namespace rhi::vulkan
{

VulkanGPUMaterial::VulkanGPUMaterial(const std::shared_ptr<VulkanShader>& shader) : m_shaderDesc(&shader->Descriptor())
{
    if (s_descriptorPool == nullptr)
    {
        AllocateDescriptorPool();
    }

    const auto layout = shader->Layout();

    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = s_descriptorPool;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &layout;

    const auto res = vkAllocateDescriptorSets(VulkanDevice::s_ctx.m_device, &allocInfo, &m_descriptorSet);

    RHI_ASSERT(res == VK_SUCCESS);
}

VulkanGPUMaterial::~VulkanGPUMaterial()
{
}

// TODO: Add validation for texture slots from reflection
void VulkanGPUMaterial::SetTexture(const std::shared_ptr<Texture>& texture, uint8_t slot, uint8_t mipLevel)
{
    m_dirty = true;

    TextureInfo info;
    info.m_texture = texture;
    info.m_slot = slot;
    info.m_mipLevel = mipLevel;

    m_texturesToSync.emplace_back(info);
}

// TODO: Add validation for buffer slots from reflection
void VulkanGPUMaterial::SetBuffer(const std::shared_ptr<Buffer>& buffer, uint8_t slot, ShaderStage stage, int offset)
{
    RHI_ASSERT_WITH_MESSAGE(buffer->Descriptor().m_size % VulkanDevice::s_ctx.m_instance->m_parameters.m_minUniformBufferAlignment == 0, fmt::format("GPU Buffer size must be multiple of {}", VulkanDevice::s_ctx.m_instance->m_parameters.m_minUniformBufferAlignment).c_str());

    m_dirty = true;

    BufferInfo info;
    info.m_buffer = buffer;
    info.m_slot = slot;
    info.m_offset = offset;
    info.m_stage = stage;

    m_buffersToSync.emplace_back(info);
}

void VulkanGPUMaterial::Sync()
{
    if (!m_dirty)
    {
        return;
    }
    m_dirty = false;

    eastl::vector<VkWriteDescriptorSet> writeDescriptorSets;
    eastl::vector<VkDescriptorBufferInfo> bufferInfos;

    writeDescriptorSets.reserve(m_buffersToSync.size() + 1);
    bufferInfos.reserve(m_buffersToSync.size() + 1);

    uint32_t i = 0;
    for (auto& buffer : m_buffersToSync)
    {
        if (buffer.m_buffer.expired())
        {
            rhi::log::warning("Expired buffer ptr in slot {} for shader '{}'", buffer.m_slot, m_shaderDesc->m_name);
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
        descriptorWrite.pBufferInfo = &bufferInfos[i];

        writeDescriptorSets.emplace_back(descriptorWrite);
        i++;
    }

    eastl::vector<VkDescriptorImageInfo> textureInfos;
    // To prevent reallocation
    textureInfos.reserve(m_texturesToSync.size() + 1);

    for (auto& texture : m_texturesToSync)
    {
        if (texture.m_texture.expired())
        {
            rhi::log::warning("Texture in slot {} in shader '{}' is expired", texture.m_slot, m_shaderDesc->m_name);
            continue;
        }

        const auto texPtr = std::static_pointer_cast<VulkanTexture>(texture.m_texture.lock());

        VkDescriptorImageInfo imageInfo{};
        if (texPtr->Descriptor().m_format == Format::D32_SFLOAT_S8_UINT ||
            eastl::find_if(m_shaderDesc->m_reflection.m_storageImages.begin(), 
                m_shaderDesc->m_reflection.m_storageImages.end(), [&texture](const auto& info) { return info.m_slot == texture.m_slot; })
            != m_shaderDesc->m_reflection.m_storageImages.end())
        {
            imageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
        }
        else
        {
            imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        }

        // TODO: Here we probably need to add some index somewhere to represent which part of the texture we want to get
        imageInfo.imageView = texPtr->ImageView(texture.m_mipLevel);

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

void VulkanGPUMaterial::Destroy()
{
    vkDestroyDescriptorPool(VulkanDevice::s_ctx.m_device, s_descriptorPool, nullptr);
}

void VulkanGPUMaterial::AllocateDescriptorPool()
{
    VkDescriptorPoolSize bufferPoolSize{};
    bufferPoolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    bufferPoolSize.descriptorCount = C_DESCRIPTOR_POOL_SIZE;

    VkDescriptorPoolSize texturePoolSize{};
    texturePoolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    texturePoolSize.descriptorCount = C_DESCRIPTOR_POOL_SIZE;

    VkDescriptorPoolSize imageStoragePoolSize{};
    imageStoragePoolSize.type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    imageStoragePoolSize.descriptorCount = C_DESCRIPTOR_POOL_SIZE;

    VkDescriptorPoolSize storageBufferPoolSize{};
    storageBufferPoolSize.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    storageBufferPoolSize.descriptorCount = C_DESCRIPTOR_POOL_SIZE;

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
    poolInfo.maxSets = C_DESCRIPTOR_POOL_SIZE;

    RHI_ASSERT(vkCreateDescriptorPool(VulkanDevice::s_ctx.m_device, &poolInfo, nullptr, &s_descriptorPool) == VK_SUCCESS);
}

VkDescriptorType VulkanGPUMaterial::DescriptorType(uint8_t slot)
{
    const auto texIt = eastl::find_if(m_shaderDesc->m_reflection.m_textures.begin(), 
        m_shaderDesc->m_reflection.m_textures.end(), [slot](const auto& info) { return info.m_slot == slot; });

    const auto storageImgIt = eastl::find_if(m_shaderDesc->m_reflection.m_storageImages.begin(),
        m_shaderDesc->m_reflection.m_storageImages.end(), [slot](const auto& info) { return info.m_slot == slot; });

    if (texIt != m_shaderDesc->m_reflection.m_textures.end())
    {
        return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    }

    if (storageImgIt != m_shaderDesc->m_reflection.m_storageImages.end())
    {
        return VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    }

    RHI_ASSERT(false);
    return VK_DESCRIPTOR_TYPE_MAX_ENUM;
}

} // rhi::vulkan
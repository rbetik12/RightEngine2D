#include "VulkanRendererState.hpp"
#include "VulkanGraphicsPipeline.hpp"
#include "VulkanBuffer.hpp"
#include "VulkanTexture.hpp"
#include "VulkanSampler.hpp"
#include "Shader.hpp"

using namespace RightEngine;

void VulkanRendererState::OnUpdate(const std::shared_ptr<GraphicsPipeline>& pipeline)
{
    for (auto& [bufferRef, buffer] : buffers)
    {
        if (buffer->IsSyncNeeded())
        {
            buffer->SetNeedToSync(false);
        }
    }

    if (descriptorSet)
    {
        if (isSyncNeeded)
        {
            std::vector<VkWriteDescriptorSet> writeDescriptorSets;
            std::vector<VkDescriptorBufferInfo> bufferInfos;
            std::vector<VkDescriptorImageInfo> textureInfos;

            bufferInfos.reserve(buffersToSync.size());
            textureInfos.reserve(texturesToSync.size());

            for (const auto& [bufferRef, bufferPtr] : buffersToSync)
            {
                if (!bufferPtr.expired())
                {
                    if (bufferPtr.lock()->GetDescriptor().type == BufferType::CONSTANT)
                    {
                        continue;
                    }
                    const auto buffer = bufferPtr.lock();
                    VkDescriptorBufferInfo bufferInfo{};
                    bufferInfo.buffer = std::static_pointer_cast<VulkanBuffer>(buffer)->GetBuffer();
                    const auto& offsetRef = offsets.find(bufferRef);
                    if (offsetRef == offsets.end())
                    {
                        bufferInfo.offset = 0;
                        bufferInfo.range = buffer->GetDescriptor().size;
                    }
                    else
                    {
                        const size_t offset = offsetRef->second.offset;
                        const size_t stride = offsetRef->second.stride;
                        const size_t minDeviceOffset =  Device::Get()->GetInfo().minUniformBufferOffsetAlignment;
                        R_CORE_ASSERT(offset % minDeviceOffset == 0, "");
                        bufferInfo.offset = offset;
                        bufferInfo.range = stride;
                    }

                    bufferInfos.push_back(bufferInfo);

                    VkWriteDescriptorSet descriptorWrite{};
                    descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                    descriptorWrite.dstSet = descriptorSet;
                    descriptorWrite.dstBinding = bufferRef.slot;
                    descriptorWrite.dstArrayElement = 0;
                    descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                    descriptorWrite.descriptorCount = 1;
                    descriptorWrite.pBufferInfo = &bufferInfos.back();

                    writeDescriptorSets.push_back(descriptorWrite);
                }
            }

            for (const auto& [slot, texturePtr] : texturesToSync)
            {
                if (!texturePtr.expired())
                {
                    const auto texture = std::static_pointer_cast<VulkanTexture>(texturePtr.lock());
                    VkDescriptorImageInfo imageInfo{};
                    if (texture->GetSpecification().format == Format::D32_SFLOAT_S8_UINT)
                    {
                        imageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
                    }
                    else
                    {
                        imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                    }
                    imageInfo.imageView = texture->GetImageView();
                    imageInfo.sampler = std::static_pointer_cast<VulkanSampler>(texture->GetSampler())->GetSampler();

                    textureInfos.push_back(imageInfo);

                    VkWriteDescriptorSet descriptorWrite{};
                    descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                    descriptorWrite.dstSet = descriptorSet;
                    descriptorWrite.dstBinding = slot;
                    descriptorWrite.dstArrayElement = 0;
                    descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                    descriptorWrite.descriptorCount = 1;
                    descriptorWrite.pImageInfo = &textureInfos.back();

                    writeDescriptorSets.push_back(descriptorWrite);
                }
            }

            vkUpdateDescriptorSets(VK_DEVICE()->GetDevice(),
                                   writeDescriptorSets.size(),
                                   writeDescriptorSets.data(), 0,
                                   nullptr);

            isSyncNeeded = false;
            buffersToSync.clear();
            texturesToSync.clear();
        }
    }
    else
    {
        const auto& shaderDescriptor = pipeline->GetPipelineDescriptor().shader->GetShaderProgramDescriptor();

        VkDescriptorPoolSize bufferPoolSize{};
        bufferPoolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        bufferPoolSize.descriptorCount = shaderDescriptor.reflection.buffers.size();

        VkDescriptorPoolSize texturePoolSize{};
        texturePoolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        texturePoolSize.descriptorCount = shaderDescriptor.reflection.textures.size();
        
        if (bufferPoolSize.descriptorCount == 0 && texturePoolSize.descriptorCount == 0)
        {
            return;
        }

        std::vector<VkDescriptorPoolSize> poolSizes;
        if (bufferPoolSize.descriptorCount > 0)
        {
            poolSizes.push_back(bufferPoolSize);
        }
        else if (texturePoolSize.descriptorCount > 0)
        {
            poolSizes.push_back(texturePoolSize);
        }

        R_CORE_ASSERT(!poolSizes.empty(), "");

        VkDescriptorPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.poolSizeCount = poolSizes.size();
        poolInfo.pPoolSizes = poolSizes.data();
        poolInfo.maxSets = 1;

        if (vkCreateDescriptorPool(VK_DEVICE()->GetDevice(), &poolInfo, nullptr,
                                   &descriptorPool) != VK_SUCCESS)
        {
            R_CORE_ASSERT(false, "failed to create descriptor pool!");
        }

        std::vector<VkDescriptorSetLayout> layouts(1,std::static_pointer_cast<VulkanGraphicsPipeline>(
                                                           pipeline)->GetDescriptorSetLayout());
        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = descriptorPool;
        allocInfo.descriptorSetCount = 1;
        allocInfo.pSetLayouts = layouts.data();

        if (vkAllocateDescriptorSets(VK_DEVICE()->GetDevice(), &allocInfo, &descriptorSet) != VK_SUCCESS)
        {
            R_CORE_ASSERT(false, "failed to allocate descriptor sets!");
        }

        OnUpdate(pipeline);
    }
}

VulkanRendererState::~VulkanRendererState()
{
    if (descriptorPool)
    {
        vkDestroyDescriptorPool(VK_DEVICE()->GetDevice(), descriptorPool, nullptr);
    }
}


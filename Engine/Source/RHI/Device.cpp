#include "Device.hpp"
#include "Application.hpp"
#include "Vulkan/VulkanDevice.hpp"

using namespace RightEngine;

std::shared_ptr<Device> Device::device{ nullptr };

std::shared_ptr<Device> Device::Get(const std::shared_ptr<RenderingContext>& ctx,
                                    const std::shared_ptr<Surface>& surface)
{
    R_CORE_ASSERT(!(device == nullptr && ctx == nullptr), "");
    R_CORE_ASSERT(!(device == nullptr && surface == nullptr), "");

    if (device == nullptr)
    {
        device = std::make_shared<VulkanDevice>(ctx, surface);
    }
    return device;
}

std::shared_ptr<Buffer> Device::CreateBuffer(const BufferDescriptor& bufferDescriptor, const void* data)
{
    return std::shared_ptr<Buffer>();
}

size_t Device::GetAlignedGPUDataSize(size_t dataSize) const
{
    if (dataSize % properties.minUniformBufferOffsetAlignment == 0)
    {
        return dataSize;
    }

    const size_t offsetsCount = dataSize / properties.minUniformBufferOffsetAlignment + 1;
    return offsetsCount * properties.minUniformBufferOffsetAlignment;
}

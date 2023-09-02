#pragma once

#include "RenderingContext.hpp"
#include "BufferDescriptor.hpp"
#include "CommandBufferDescriptor.hpp"
#include "ShaderProgramDescriptor.hpp"
#include "GraphicsPipelineDescriptor.hpp"
#include "TextureDescriptor.hpp"
#include "SamplerDescriptor.hpp"

#include <memory>

#define VK_DEVICE() std::static_pointer_cast<VulkanDevice>(Device::Get())

namespace RightEngine
{
    class Buffer;
    class CommandBuffer;
    class Shader;
    class Surface;
    class GraphicsPipeline;
    class Texture;
    class Sampler;

    struct DeviceProperties
    {
        size_t minUniformBufferOffsetAlignment;
    };

    class Device : public std::enable_shared_from_this<Device>
    {
    public:
        const DeviceProperties& GetInfo() const
        { return properties; }

        size_t GetAlignedGPUDataSize(size_t dataSize) const;

        virtual ~Device() = default;

        virtual std::shared_ptr<Buffer> CreateBuffer(const BufferDescriptor& bufferDescriptor, const void* data) = 0;
        virtual std::shared_ptr<CommandBuffer> CreateCommandBuffer(const CommandBufferDescriptor& descriptor) = 0;
        virtual std::shared_ptr<Shader> CreateShader(const ShaderProgramDescriptor& shaderProgramDescriptor) = 0;
        virtual std::shared_ptr<GraphicsPipeline> CreateGraphicsPipeline(const GraphicsPipelineDescriptor& descriptor,
                                                                         const RenderPassDescriptor& renderPassDescriptor) = 0;
        virtual std::shared_ptr<Texture> CreateTexture(const TextureDescriptor& descriptor,
                                                       const std::vector<uint8_t>& data) = 0;

        virtual std::shared_ptr<Sampler> CreateSampler(const SamplerDescriptor& descriptor) = 0;

        Device(const Device& other) = delete;
        Device& operator=(const Device& other) = delete;
        Device(Device&& other) = delete;
        Device& operator=(Device&& other) = delete;

        static std::shared_ptr<Device> Get(const std::shared_ptr<RenderingContext>& ctx = nullptr,
                                           const std::shared_ptr<Surface>& surface = nullptr);

    protected:
        Device(const std::shared_ptr<RenderingContext>& context, const std::shared_ptr<Surface>& surface)
        {}

        static std::shared_ptr<Device> device;
        DeviceProperties properties{};
    };
}

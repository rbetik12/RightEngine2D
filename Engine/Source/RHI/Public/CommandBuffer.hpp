#pragma once

#include "Device.hpp"
#include "CommandBufferDescriptor.hpp"

namespace RightEngine
{
    class CommandBuffer
    {
    public:
        virtual ~CommandBuffer() = default;

        virtual void Enqueue(std::function<void(CommandBuffer*)>&& command) = 0;

        virtual void Execute() = 0;

    protected:
        CommandBuffer(const std::shared_ptr<Device>& device, const CommandBufferDescriptor& descriptor) : descriptor(descriptor)
        {}

        CommandBufferDescriptor descriptor;
    };
}

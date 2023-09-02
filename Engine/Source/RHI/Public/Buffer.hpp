#pragma once

#include "Device.hpp"
#include "BufferDescriptor.hpp"

namespace RightEngine
{
    class Buffer
    {
    public:
        const BufferDescriptor& GetDescriptor() const
        { return descriptor; }

        virtual ~Buffer() = default;

        virtual void* Map() const = 0;
        virtual void UnMap() const = 0;

        void SetData(const void* data, size_t size, size_t offset = 0) const;

        void SetNeedToSync(bool aIsSyncNeeded)
        { isSyncNeeded = aIsSyncNeeded; }

        bool IsSyncNeeded() const
        { return isSyncNeeded; }

    protected:
        Buffer(const std::shared_ptr<Device>& device, const BufferDescriptor& bufferDescriptor, const void* data) : descriptor(bufferDescriptor)
        {}

        BufferDescriptor descriptor;
        bool isSyncNeeded{ false };
    };
}

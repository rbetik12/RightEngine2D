#pragma once

#include <RHI/Config.hpp>
#include <RHI/Assert.hpp>
#include <RHI/BufferDescriptor.hpp>
#include <Core/Type.hpp>

namespace rhi
{

class RHI_API Buffer : public core::NonCopyable
{
public:
    Buffer(const BufferDescriptor& descriptor) : m_descriptor(descriptor)
    {}

    virtual ~Buffer() = default;

    virtual void*    Map() const = 0;
    virtual void     UnMap() const = 0;

    inline void      CopyToBuffer(const void* ptr, size_t size)
    {
        std::lock_guard l(m_copyMutex);

        RHI_ASSERT(size <= m_descriptor.m_size);

        auto* dst = Map();
        std::memcpy(dst, ptr, size);
        UnMap();
    }

    const BufferDescriptor& Descriptor() const
    { return m_descriptor; }

protected:
    BufferDescriptor    m_descriptor;
    std::mutex          m_copyMutex;
};

}

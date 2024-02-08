#pragma once

#include <RHI/Types.hpp>

namespace rhi
{
    enum class BufferType : uint8_t
    {
        VERTEX =        Bit(0),
        TRANSFER_SRC =  Bit(1),
        TRANSFER_DST =  Bit(2),
        INDEX =         Bit(3),
        UNIFORM =       Bit(4),
        CONSTANT =      Bit(5),
    };

    struct BufferDescriptor
    {
        std::string     m_name;
        uint32_t        m_size;
        BufferType      m_type;
        MemoryType      m_memoryType;
    };
}
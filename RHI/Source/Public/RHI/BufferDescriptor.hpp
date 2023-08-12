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
        size_t size;
        BufferType type;
        MemoryType memoryType;
    };
}
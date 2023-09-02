#pragma once

#include "Types.hpp"

namespace RightEngine
{
    enum class BufferType
    {
        VERTEX = BIT(0),
        TRANSFER_SRC = BIT(1),
        TRANSFER_DST = BIT(2),
        INDEX = BIT(3),
        UNIFORM = BIT(4),
        CONSTANT = BIT(5),
    };

    struct BufferDescriptor
    {
        size_t size;
        BufferType type;
        MemoryType memoryType;
    };
}

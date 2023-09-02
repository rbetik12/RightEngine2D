#pragma once

namespace RightEngine
{
    enum class CommandBufferType
    {
        GRAPHICS,
        COMPUTE
    };

    struct CommandBufferDescriptor
    {
        CommandBufferType type;
    };
}
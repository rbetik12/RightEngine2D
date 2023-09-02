#include "Buffer.hpp"

using namespace RightEngine;

void Buffer::SetData(const void* data, size_t size, size_t offset) const
{
    auto bufferPtr = static_cast<uint8_t*>(Map());
    memcpy(bufferPtr + offset, data, size);
    UnMap();
}

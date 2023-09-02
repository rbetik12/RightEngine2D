#include "VertexBufferLayout.hpp"

unsigned int RightEngine::VertexBufferElement::GetSizeOfType(Format format)
{
    switch (format) {
        case Format::BGRA8_SRGB:
            return 1;
        case Format::RG32_SFLOAT:
            return 8;
        case Format::R32_SFLOAT:
            return 4;
        case Format::RGB32_SFLOAT:
            return 12;
        case Format::RGBA32_SFLOAT:
            return 16;
    }
    assert(false);
    return 0;
}


#pragma once

#include <cstdint>
#include <limits>

namespace core::math
{

constexpr float PI = 3.14159f;

inline uint32_t roundUpToNextHighestPowerOfTwo(uint32_t value)
{
    value--;
    value |= value >> 1;
    value |= value >> 2;
    value |= value >> 4;
    value |= value >> 8;
    value |= value >> 16;
    value++;
    return value;
}

inline bool almostEqual(float a, float b, float epsilon = std::numeric_limits<float>::epsilon())
{
    return std::abs(a - b) <= epsilon;
}

} // core::math
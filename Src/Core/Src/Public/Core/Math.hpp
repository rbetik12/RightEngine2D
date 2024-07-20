#pragma once

#include <Core/Assert.hpp>
#include <glm/glm.hpp>
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
    return glm::abs(a - b) <= epsilon;
}

inline uint32_t roundToDivisible(uint32_t num, uint32_t divisor)
{
    CORE_ASSERT(divisor != 0);

    const uint32_t remainder = num % divisor;

    if (remainder == 0) 
    {
        return num;
    }

    return num + (divisor - remainder);
}

} // core::math
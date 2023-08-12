#pragma once

#include <EASTL/functional.h>

namespace core::hash
{
    template<class T>
    inline void CombineHash(size_t& seed, const T& value)
    {
        eastl::hash<T> hash;
        seed ^= hash(value) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    }
}
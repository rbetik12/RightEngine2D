#pragma once

#include <cstdint>

namespace core::math
{
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
}
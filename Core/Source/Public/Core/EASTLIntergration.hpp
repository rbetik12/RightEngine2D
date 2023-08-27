#pragma once

#include <EASTL/functional.h>
#include <string>

namespace eastl
{
	template<>
	struct hash<std::string>
	{
		size_t operator()(const std::string& str) const
		{
			return str.size();
		}
	};
}
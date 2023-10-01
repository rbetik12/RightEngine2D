#pragma once

#include <Core/Hash.hpp>
#include <EASTL/functional.h>
#include <string>
#include <typeindex>

namespace eastl
{

template<>
struct hash<std::string>
{
	size_t operator()(const std::string& str) const
	{
		return core::hash::HashString(str);
	}
};

template<>
struct hash<std::type_index>
{
	size_t operator()(const std::type_index& tIdx) const
	{
		return tIdx.hash_code();
	}
};

} // namespace eastl
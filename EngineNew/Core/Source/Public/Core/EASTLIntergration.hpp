#pragma once

#include <Core/Hash.hpp>
#include <EASTL/functional.h>
#include <EASTL/vector.h>
#include <EASTL/unordered_map.h>
#include <rttr/type>
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

template<>
struct hash<rttr::type>
{
	size_t operator()(rttr::type type) const
	{
		return type.get_id();
	}
};

} // namespace eastl
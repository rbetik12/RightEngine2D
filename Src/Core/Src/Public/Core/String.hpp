#pragma once

#include <Core/Config.hpp>
#include <string>

namespace core::string
{

std::wstring CORE_API Convert(std::string_view str);

std::string CORE_API BytesToHumanReadable(size_t size);

} // namespace core::string
#include <Core/String.hpp>

#ifdef R_WIN32
#include <windows.h>
#endif

namespace core::string
{

std::wstring Convert(std::string_view str)
{
#ifdef R_WIN32
	const auto count = MultiByteToWideChar(CP_UTF8, 0, str.data(), static_cast<int>(str.size()), nullptr, 0);
	std::wstring result(count, L'\0');
	MultiByteToWideChar(CP_UTF8, 0, str.data(), static_cast<int>(str.size()), result.data(), static_cast<int>(result.size()));
	return result;
#else
	static_assert(false);
	return L""
#endif
}

}
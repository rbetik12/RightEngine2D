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

std::string BytesToHumanReadable(size_t size)
{
	static constexpr std::string_view units[] = { "B", "KB", "MB", "GB", "TB" };
	static constexpr int length = static_cast<int>(std::size(units));

	int i = 0;

	auto fSize = static_cast<float>(size);

	if (size >= 1024)
	{
		for (i = 0; (size / 1024) > 0 && i < length - 1; i++, size /= 1024)
		{
			fSize = size / 1024.0f;
		}
	}

	return fmt::format("{:.2f} {}", fSize, units[i]);
}

}
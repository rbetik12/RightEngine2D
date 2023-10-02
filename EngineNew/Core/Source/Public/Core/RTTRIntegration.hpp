#pragma once

#include <fmt/format.h>
#include <rttr/string_view.h>

FMT_BEGIN_NAMESPACE

template <>
struct formatter<rttr::string_view> final : formatter<std::string_view>
{
    template <typename FormatContext>
    auto format(rttr::string_view value, FormatContext& ctx) const -> typename FormatContext::iterator
    {
        return formatter<std::string_view>::format(value.data(), ctx);
    }
};

FMT_END_NAMESPACE
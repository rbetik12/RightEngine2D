#pragma once

#include <EASTL/functional.h>
#include <EASTL/span.h>

namespace core::hash
{

    namespace detail
    {

        template<typename T> struct fnv1a_traits;

        template<> struct fnv1a_traits<uint64_t>
        {
            inline static constexpr uint64_t prime = 0x100000001b3;
            inline static constexpr uint64_t offset_basis = 0xcbf29ce484222325;
        };

        template<> struct fnv1a_traits<uint32_t>
        {
            inline static constexpr uint32_t prime = 0x1000193;
            inline static constexpr uint32_t offset_basis = 0x811c9dc5;
        };

        /*
         * Compile-time hash function
         * https://en.wikipedia.org/wiki/Fowler-Noll-Vo_hash_function
         * http://www.isthe.com/chongo/tech/comp/fnv/
         */
        template<typename T, typename TRange>
        constexpr T fnv1a(TRange&& range) noexcept
        {
            T result = fnv1a_traits<T>::offset_basis;
            for (const auto element : range)
            {
                result = (result ^ static_cast<uint8_t>(element)) * fnv1a_traits<T>::prime;
            }
            return result;
        }

    } // namespace detail

    constexpr uint64_t HashBytes(eastl::span<const uint8_t> buf) noexcept
    {
        return detail::fnv1a<uint64_t>(buf);
    }

    constexpr uint32_t HashBytes32(eastl::span<const uint8_t> buf) noexcept
    {
        return detail::fnv1a<uint32_t>(buf);
    }

    constexpr uint64_t HashString(std::string_view str) noexcept
    {
        return detail::fnv1a<uint64_t>(str);
    }

    constexpr uint32_t HashString32(std::string_view str) noexcept
    {
        return detail::fnv1a<uint32_t>(str);
    }

    constexpr uint64_t operator "" _hash(const char* str, size_t len) noexcept
    {
        return detail::fnv1a<uint64_t>(std::string_view{ str, len });
    }

    constexpr uint32_t operator "" _hash32(const char* str, size_t len) noexcept
    {
        return detail::fnv1a<uint32_t>(std::string_view{ str, len });
    }

    template<class T>
    inline void CombineHash(size_t& seed, const T& value)
    {
        eastl::hash<T> hash;
        seed ^= hash(value) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    }

} // namespace core::hash
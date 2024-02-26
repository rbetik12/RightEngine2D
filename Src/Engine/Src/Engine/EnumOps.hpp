#pragma once

namespace engine::flags
{

template<typename Enum, typename... Rest> Enum bitwiseOr(const Enum l, const Enum r, const Rest... rest)
{
    return static_cast<Enum>((static_cast<std::underlying_type_t<Rest>>(rest) | ... | (static_cast<std::underlying_type_t<Enum>>(l) | static_cast<std::underlying_type_t<Enum>>(r))));
}

template<typename Enum, typename... Rest> Enum bitwiseAnd(const Enum l, const Enum r, const Rest... rest)
{
    return static_cast<Enum>((static_cast<std::underlying_type_t<Rest>>(rest) & ... & (static_cast<std::underlying_type_t<Enum>>(l) & static_cast<std::underlying_type_t<Enum>>(r))));
}

template<typename Enum> bool has(const Enum l, const Enum r)
{
    return bitwiseAnd(l, r) == r;
}

} // namespace engine::flags

#define ENGINE_DEFINE_BITWISE_OP_BINARY(Enum, Op, OpAssign)                                                                            \
      inline constexpr Enum operator Op(const Enum l, const Enum r)                                                                    \
      {                                                                                                                                \
        return static_cast<Enum>(static_cast<std::underlying_type_t<Enum>>(l) Op static_cast<std::underlying_type_t<Enum>>(r));        \
      }                                                                                                                                \
      inline constexpr Enum& operator OpAssign(Enum& l, const Enum r)                                                                \
      {                                                                                                                                \
        return l = l Op r;                                                                                                            \
      }                                                                                                                                \

#define ENGINE_DEFINE_BITWISE_OPS(Enum)                                                                                                \
      static_assert(std::is_enum_v<Enum>, "This macro should only be used for enums.");                                                \
      static_assert(std::is_unsigned_v<std::underlying_type_t<Enum>>, "Should use an unsigned underlying type for flags.");            \
      ENGINE_DEFINE_BITWISE_OP_BINARY(Enum, |, |=)                                                                                    \
      ENGINE_DEFINE_BITWISE_OP_BINARY(Enum, &, &=)                                                                                    \
      ENGINE_DEFINE_BITWISE_OP_BINARY(Enum, ^, ^=)                                                                                    \
      inline constexpr Enum operator~(const Enum v)                                                                                    \
      {                                                                                                                                \
        return static_cast<Enum>(~static_cast<std::underlying_type_t<Enum>>(v));                                                    \
      }                                                                                                                                \
      inline constexpr bool operator!(const Enum v)                                                                                    \
      {                                                                                                                                \
        return static_cast<std::underlying_type_t<Enum>>(v) == 0;                                                                    \
      }
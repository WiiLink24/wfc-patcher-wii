#pragma once

#include "wwfcLibC.hpp"

namespace wwfc
{

template <class SizeType, class ValueType>
class [[gnu::packed]] Enum
{
    static_assert(std::is_enum_v<ValueType>, "ValueType must be an enum type");
    static_assert(std::is_integral_v<SizeType>, "SizeType must be an integral type");

    SizeType m_underlying_value;

public:
    constexpr Enum()
      : m_underlying_value(0)
    {
    }

    constexpr Enum(ValueType value)
      : m_underlying_value(static_cast<SizeType>(value))
    {
    }

    constexpr Enum(const Enum& other)
      : m_underlying_value(other.m_underlying_value)
    {
    }

    template <class OtherSizeType>
    constexpr Enum(const Enum<OtherSizeType, ValueType>& other)
      : m_underlying_value(static_cast<SizeType>(other))
    {
    }

    constexpr operator ValueType() const
    {
        return static_cast<ValueType>(m_underlying_value);
    }

    constexpr Enum& operator=(ValueType value)
    {
        m_underlying_value = static_cast<SizeType>(value);
        return *this;
    }

    template <class OtherSizeType>
    constexpr Enum& operator=(const Enum<OtherSizeType, ValueType>& other)
    {
        m_underlying_value = static_cast<SizeType>(other);
        return *this;
    }

    constexpr bool operator==(ValueType value) const
    {
        return static_cast<ValueType>(m_underlying_value) == value;
    }

    template <class OtherSizeType>
    constexpr bool operator==(const Enum<OtherSizeType, ValueType>& other) const
    {
        return m_underlying_value == static_cast<SizeType>(other);
    }

    constexpr bool operator<(ValueType value) const
    {
        return static_cast<ValueType>(m_underlying_value) < value;
    }

    template <class OtherSizeType>
    constexpr bool operator<(const Enum<OtherSizeType, ValueType>& other) const
    {
        return m_underlying_value < static_cast<SizeType>(other);
    }

    constexpr bool operator<=(ValueType value) const
    {
        return static_cast<ValueType>(m_underlying_value) <= value;
    }

    template <class OtherSizeType>
    constexpr bool operator<=(const Enum<OtherSizeType, ValueType>& other) const
    {
        return m_underlying_value <= static_cast<SizeType>(other);
    }

    constexpr bool operator!() const
    {
        return m_underlying_value == 0;
    }

    explicit constexpr operator char() const
    {
        return static_cast<char>(m_underlying_value);
    }

    explicit constexpr operator signed char() const
    {
        return static_cast<signed char>(m_underlying_value);
    }

    explicit constexpr operator unsigned char() const
    {
        return static_cast<unsigned char>(m_underlying_value);
    }

    explicit constexpr operator wchar_t() const
    {
        return static_cast<wchar_t>(m_underlying_value);
    }

    explicit constexpr operator char16_t() const
    {
        return static_cast<char16_t>(m_underlying_value);
    }

    explicit constexpr operator char32_t() const
    {
        return static_cast<char32_t>(m_underlying_value);
    }

    explicit constexpr operator short() const
    {
        return static_cast<short>(m_underlying_value);
    }

    explicit constexpr operator unsigned short() const
    {
        return static_cast<unsigned short>(m_underlying_value);
    }

    explicit constexpr operator int() const
    {
        return static_cast<int>(m_underlying_value);
    }

    explicit constexpr operator unsigned int() const
    {
        return static_cast<unsigned int>(m_underlying_value);
    }

    explicit constexpr operator long() const
    {
        return static_cast<long>(m_underlying_value);
    }

    explicit constexpr operator unsigned long() const
    {
        return static_cast<unsigned long>(m_underlying_value);
    }

    explicit constexpr operator long long() const
    {
        return static_cast<long long>(m_underlying_value);
    }

    explicit constexpr operator unsigned long long() const
    {
        return static_cast<unsigned long long>(m_underlying_value);
    }

#if defined(__SIZEOF_INT128__)
    explicit constexpr operator __int128() const
    {
        return static_cast<__int128>(m_underlying_value);
    }

    explicit constexpr operator unsigned __int128() const
    {
        return static_cast<unsigned __int128>(m_underlying_value);
    }
#endif
};

} // namespace wwfc

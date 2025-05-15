#pragma once

#include <wwfcUtil.h>

#ifdef WWFC_HAVE_LIBC
#  include <stddef.h>
#  include <string.h>

namespace wwfc::std
{

using ::size_t;

using ::memchr;
using ::memcmp;
using ::memcpy;
using ::memset;
using ::strchr;
using ::strcmp;
using ::strcpy;
using ::strlen;
using ::strncmp;
using ::strncpy;

} // namespace wwfc::std

#else // !WWFC_HAVE_LIBC
namespace wwfc::std
{

#  if defined(__GNUC__) && !defined(__clang__)
using size_t = unsigned int;
#  else
using size_t = unsigned long;
#  endif

extern "C" {
void* memcpy(void* __restrict dest, const void* __restrict src, size_t n);
void* memset(void* s, int c, size_t n);
int memcmp(const void* s1, const void* s2, size_t n);
void* memchr(const void* s, int c, size_t n);
size_t strlen(const char* s);
int strcmp(const char* s1, const char* s2);
int strncmp(const char* s1, const char* s2, size_t n);
char* strchr(const char* s, int c);
char* strcpy(char* __restrict dst, const char* __restrict src);
char* strncpy(char* __restrict dst, const char* __restrict src, size_t n);
} // extern "C"

#  ifndef NULL
#    define NULL 0
#  endif

} // namespace wwfc::std
#endif // !WWFC_HAVE_LIBC

#ifdef WWFC_HAVE_LIBCPP
#  include <algorithm>
#  include <array>
#  include <bit>
#  include <iterator>

namespace wwfc::std
{

using ::std::array;
using ::std::countr_one;
using ::std::difference;
using ::std::max;
using ::std::min;
using ::std::size;

} // namespace wwfc::std

#else // !WWFC_HAVE_LIBCPP
namespace wwfc::std
{

template <class T>
constexpr const T& min(const T& l, const T& r)
{
    return (r < l) ? r : l;
}

template <class T>
constexpr const T& max(const T& l, const T& r)
{
    return (l < r) ? r : l;
}

template <class C>
constexpr auto size(const C& c) -> decltype(c.size())
{
    return c.size();
}

template <class T, std::size_t N>
constexpr std::size_t size(const T (&array)[N]) noexcept
{
    return N;
}

template <class T>
constexpr auto distance(T first, T last) -> decltype(last - first)
{
    return last - first;
}

template <typename T>
constexpr int countr_one(T x)
{
    int count = 0;
    while (x & 1) {
        ++count;
        x >>= 1;
    }
    return count;
}

template <class T, std::size_t N>
class array
{
public:
    using value_type = T;
    using size_type = std::size_t;
    // Missing: difference_type
    using reference = T&;
    using const_reference = const T&;
    using pointer = T*;
    using const_pointer = const T*;

    // Missing: iterator, const_iterator, reverse_iterator,
    // const_reverse_iterator

    constexpr reference at(size_type index) noexcept
    {
        // Missing: std::out_of_range exception
        return m_data[index];
    }

    constexpr const_reference at(size_type index) const noexcept
    {
        // Missing: std::out_of_range exception
        return m_data[index];
    }

    constexpr reference operator[](size_type index) noexcept
    {
        return m_data[index];
    }

    constexpr const_reference operator[](size_type index) const noexcept
    {
        return m_data[index];
    }

    constexpr reference front() noexcept
    {
        return m_data[0];
    }

    constexpr const_reference front() const noexcept
    {
        return m_data[0];
    }

    constexpr reference back() noexcept
    {
        return m_data[N - 1];
    }

    constexpr const_reference back() const noexcept
    {
        return m_data[N - 1];
    }

    constexpr pointer data() noexcept
    {
        return m_data;
    }

    constexpr const_pointer data() const noexcept
    {
        return m_data;
    }

    constexpr pointer begin() noexcept
    {
        return m_data;
    }

    constexpr const_pointer begin() const noexcept
    {
        return cbegin();
    }

    constexpr const_pointer cbegin() const noexcept
    {
        return m_data;
    }

    constexpr pointer end() noexcept
    {
        return m_data + N;
    }

    constexpr const_pointer end() const noexcept
    {
        return cend();
    }

    constexpr const_pointer cend() const noexcept
    {
        return m_data + N;
    }

    // TODO: rbegin, crbegin, rend, crend

    constexpr bool empty() const noexcept
    {
        return N == 0;
    }

    constexpr size_type size() const noexcept
    {
        return N;
    }

    constexpr size_type max_size() const noexcept
    {
        return N;
    }

    constexpr void fill(const T& value) noexcept
    {
        for (size_type i = 0; i < N; ++i) {
            m_data[i] = value;
        }
    }

    // Missing: swap

public:
    value_type m_data[N] = {};
};

} // namespace wwfc::std
#endif // !WWFC_HAVE_LIBCPP
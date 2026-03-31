#pragma once

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

using size_t = decltype(sizeof(0));

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
#  include <optional>
#  include <type_traits>

namespace wwfc::std
{

using ::std::array;
using ::std::countr_one;
using ::std::difference;
using ::std::is_enum_v;
using ::std::is_same_v;
using ::std::max;
using ::std::min;
using ::std::optional;
using ::std::remove_cvref_t;
using ::std::size;

} // namespace wwfc::std

#else // !WWFC_HAVE_LIBCPP
namespace wwfc
{
struct placement_new {
    void* ptr;
};
} // namespace wwfc

constexpr void* operator new(wwfc::std::size_t size, wwfc::placement_new pn)
{
    return pn.ptr;
}

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

template <class T>
concept is_enum_v = __is_enum(T);

template <class T, class U>
concept is_same_v = __is_same_as(T, U);

template <class T>
struct identity {
    using type = T;
};

template <class T>
struct remove_reference : identity<T> {
};

template <class T>
struct remove_reference<T&> : identity<T> {
};

template <class T>
struct remove_reference<T&&> : identity<T> {
};

template <class T>
struct remove_const : identity<T> {
};

template <class T>
struct remove_const<const T> : identity<T> {
};

template <class T>
struct remove_volatile : identity<T> {
};

template <class T>
struct remove_volatile<volatile T> : identity<T> {
};

template <class T>
using remove_cvref_t = typename remove_const<
    typename remove_volatile<typename remove_reference<T>::type>::type>::type;

template <class T>
constexpr remove_reference<T>::type&& move(T&& t) noexcept
{
    return static_cast<remove_reference<T>::type&&>(t);
}

class nullopt_t
{
public:
    explicit constexpr nullopt_t(int) noexcept
    {
    }
};

constexpr nullopt_t nullopt{0};

template <class T>
class optional
{
public:
    using value_type = T;

    constexpr optional() noexcept
      : m_pointer(nullptr)
    {
    }

    constexpr optional(nullopt_t) noexcept
      : m_pointer(nullptr)
    {
    }

    constexpr optional(const value_type& value) noexcept
      : m_pointer(new (placement_new{m_storage}) value_type(value))
    {
    }

    constexpr optional(const optional& other) noexcept
    {
        if (other.has_value()) {
            m_pointer = new (placement_new{m_storage}) value_type(*other);
        }
    }

    constexpr ~optional() noexcept
    {
        if (m_pointer) {
            m_pointer->~value_type();
        }
    }

    constexpr bool has_value() const noexcept
    {
        return m_pointer != nullptr;
    }

    constexpr value_type& value() &
    {
        return *m_pointer;
    }

    constexpr const value_type& value() const&
    {
        return *m_pointer;
    }

    constexpr value_type&& value() &&
    {
        return move(*m_pointer);
    }

    constexpr const value_type&& value() const&&
    {
        return move(*m_pointer);
    }

    constexpr const value_type& operator*() const noexcept
    {
        return *m_pointer;
    }

    constexpr value_type& operator*() noexcept
    {
        return *m_pointer;
    }

private:
    alignas(value_type) unsigned char m_storage[sizeof(value_type)];
    value_type* m_pointer = nullptr;
};

} // namespace wwfc::std
#endif // !WWFC_HAVE_LIBCPP
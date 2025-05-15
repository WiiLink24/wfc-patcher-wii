#include "wwfcLibC.hpp"

namespace wwfc::std
{
extern "C" {

[[__gnu__::__optimize__("-fno-tree-loop-distribute-patterns"
)]] [[__gnu__::__weak__]]
void* memcpy(void* __restrict dest, const void* __restrict src, size_t n)
{
    u8* d = (u8*) dest;
    const u8* s = (const u8*) src;

    while (n-- > 0) {
        *d++ = *s++;
    }

    return dest;
}

[[__gnu__::__optimize__("-fno-tree-loop-distribute-patterns"
)]] [[__gnu__::__weak__]]
void* memset(void* s, int c, size_t n)
{
    u8* p = (u8*) s;
    for (size_t i = 0; i < n; i++) {
        p[i] = c;
    }
    return s;
}

[[__gnu__::__weak__]]
int memcmp(const void* s1, const void* s2, size_t n)
{
    const u8* su1 = (const u8*) s1;
    const u8* su2 = (const u8*) s2;

    size_t i = 0;
    for (; i < n && su1[i] == su2[i]; i++) {
    }

    return i < n ? su1[i] - su2[i] : 0;
}

[[__gnu__::__weak__]]
void* memchr(const void* s, int c, size_t n)
{
    const u8* su = (const u8*) s;

    for (size_t i = 0; i < n; i++) {
        if (su[i] == c) {
            // This function is also a const-cast
            return (void*) &su[i];
        }
    }

    return NULL;
}

[[__gnu__::__weak__]]
size_t strlen(const char* s)
{
    const char* f = s;
    while (*s != '\0') {
        s++;
    }
    return s - f;
}

[[__gnu__::__weak__]]
int strcmp(const char* s1, const char* s2)
{
    size_t i = 0;
    for (; s1[i] == s2[i]; i++) {
        if (s1[i] == '\0') {
            break;
        }
    }

    return s1[i] - s2[i];
}

[[__gnu__::__weak__]]
int strncmp(const char* s1, const char* s2, size_t n)
{
    size_t i = 0;
    for (; i < n && s1[i] == s2[i]; i++) {
        if (s1[i] == '\0') {
            break;
        }
    }

    return i < n ? s1[i] - s2[i] : 0;
}

[[__gnu__::__weak__]]
char* strchr(const char* s, int c)
{
    while (*s != '\0') {
        if (*s == c) {
            // This function is also a const-cast
            return (char*) s;
        }

        s++;
    }

    return NULL;
}

[[__gnu__::__weak__]]
char* strcpy(char* __restrict dst, const char* __restrict src)
{
    return (char*) memcpy(dst, src, strlen(src) + 1);
}

[[__gnu__::__weak__]]
char* strncpy(char* __restrict dst, const char* __restrict src, size_t n)
{
    if (n == 0) {
        return dst;
    }

    return (char*) memcpy(dst, src, std::min<std::size_t>(strlen(src) + 1, n));
}

[[__gnu__::__weak__]]
int atexit(void (*__func)(void))
{
    return 1;
}

[[__gnu__::__weak__]]
int __cxa_guard_acquire(long long int* guard)
{
    return !(*guard);
}

[[__gnu__::__weak__]]
void __cxa_guard_release(long long int* guard)
{
    *guard = 1;
}

[[__gnu__::__weak__]]
void __cxa_guard_abort(long long int* guard)
{
    *guard = 0;
}

} // extern "C"
} // namespace wwfc::std
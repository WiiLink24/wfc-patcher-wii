#pragma once

#include <array>
#include <functional>
#include <string>
#include <type_traits>
#include <wwfcAsm.h>
#include <wwfcCommon.h>

namespace wwfc::Patch
{

void ApplyPatch(u32 base, const wwfc_patch& patch);
void ApplyPatchList(u32 base, wwfc_patch* patches, u32 patchCount);

template <typename T, u32 N>
constexpr wwfc_patch Write(u8 level, u32 address, const T (&string)[N])
{
    return wwfc_patch{
        .level = level,
        .type = WWFC_PATCH_TYPE_WRITE,
        .address = address,
        .arg0p = string,
        .arg1 = sizeof(string),
    };
}

template <u32 N>
constexpr wwfc_patch WriteString(u8 level, u32 address, const char (&string)[N])
{
    return wwfc_patch{
        .level = level,
        .type = WWFC_PATCH_TYPE_WRITE,
        .address = address,
        .arg0p = string,
        .arg1 = sizeof(string),
    };
}

constexpr wwfc_patch
WriteASM(u8 level, u32 address, u32 instructionCount, auto function)
{
    return wwfc_patch{
        .level = level,
        .type = WWFC_PATCH_TYPE_WRITE,
        .address = address,
        .arg0 = u32(+function),
        .arg1 = instructionCount * sizeof(u32),
    };
}

constexpr wwfc_patch
BranchWithCTR(u8 level, u32 address, auto function, u32 tempReg = r12)
{
    return wwfc_patch{
        .level = level,
        .type = WWFC_PATCH_TYPE_BRANCH_CTR,
        .address = address,
        .arg0 = u32(+function),
        .arg1 = tempReg,
    };
}

constexpr wwfc_patch
CallWithCTR(u8 level, u32 address, auto function, u32 tempReg = r12)
{
    return wwfc_patch{
        .level = level,
        .type = WWFC_PATCH_TYPE_BRANCH_CTR_LINK,
        .address = address,
        .arg0 = u32(+function),
        .arg1 = tempReg,
    };
}

#define _WWFC_DEFINE_PATCH2(NUM)                                               \
    __attribute__((__section__(".wwfc_patch"))) wwfc_patch __wwfc_patch_##NUM[]

#define _WWFC_DEFINE_PATCH1(NUM) _WWFC_DEFINE_PATCH2(NUM)
#define WWFC_DEFINE_PATCH _WWFC_DEFINE_PATCH1(__COUNTER__)

// Silly hack to load this address through a fixup address
#define _WWFC_DEFINE_CTR_STUB2(_ADDRESS, _PROTOTYPE, _COUNTER, ...)            \
    extern int _CTR_STUB_DEST_##_COUNTER AT(_ADDRESS);                         \
    extern "C" {                                                               \
    int* _CTR_STUB_##_COUNTER = &_CTR_STUB_DEST_##_COUNTER;                    \
    }                                                                          \
                                                                               \
    __attribute__((__weak__)) _PROTOTYPE                                       \
    {                                                                          \
        asm volatile("" #__VA_ARGS__ "\n"                                      \
                     "bl 4\n"                                                  \
                     "mflr 12\n"                                               \
                     "lwz 12, _CTR_STUB_" #_COUNTER " - (. - 4)(12)\n"         \
                     "mtctr 12\n"                                              \
                     "bctr\n");                                                \
        __builtin_unreachable();                                               \
    }

#define _WWFC_DEFINE_CTR_STUB1(_ADDRESS, _PROTOTYPE, NUM, ...)                 \
    _WWFC_DEFINE_CTR_STUB2(_ADDRESS, _PROTOTYPE, NUM, __VA_ARGS__)
#define WWFC_DEFINE_CTR_STUB(_ADDRESS, _PROTOTYPE, ...)                        \
    _WWFC_DEFINE_CTR_STUB1(_ADDRESS, _PROTOTYPE, __COUNTER__, __VA_ARGS__)

} // namespace wwfc::Patch

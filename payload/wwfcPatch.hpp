#pragma once

#include "wwfcLibC.hpp"
#include <wwfcAsm.h>
#include <wwfcTypes.h>

namespace wwfc::Patch
{

template <class T0, class T1>
struct [[gnu::packed]] PatchEntry {
    static_assert(sizeof(T0) == 4 && sizeof(T1) == 4);

    u8 level; // wwfc_patch_level
    u8 type; // wwfc_patch_type
    u8 reserved[2];
    u32 address;
    T0 arg0;
    T1 arg1;
};

void ApplyPatch(u32 base, wwfc_patch& patch);
void ApplyPatchList(u32 base, wwfc_patch* patches, u32 patchCount);

template <class T, u32 N>
constexpr auto Write(u8 level, u32 address, const T (&string)[N])
{
    return PatchEntry<const T*, u32>{
        .level = level,
        .type = WWFC_PATCH_TYPE_WRITE,
        .address = address,
        .arg0 = string,
        .arg1 = sizeof(string),
    };
}

template <u32 N>
constexpr auto WriteString(u8 level, u32 address, const char (&string)[N])
{
    return PatchEntry<const char*, u32>{
        .level = level,
        .type = WWFC_PATCH_TYPE_WRITE,
        .address = address,
        .arg0 = string,
        .arg1 = sizeof(string),
    };
}

constexpr auto WritePointer(u8 level, u32 address, auto pointer)
{
    return PatchEntry<decltype(+pointer), u32>{
        .level = level,
        .type = WWFC_PATCH_TYPE_WRITE_POINTER,
        .address = address,
        .arg0 = +pointer,
    };
}

constexpr auto
WriteASM(u8 level, u32 address, u32 instructionCount, auto function)
{
    return PatchEntry<decltype(+function), u32>{
        .level = level,
        .type = WWFC_PATCH_TYPE_WRITE,
        .address = address,
        .arg0 = +function,
        .arg1 = instructionCount * sizeof(u32),
    };
}

constexpr auto
BranchWithCTR(u8 level, u32 address, auto function, u32 tempReg = 12)
{
    return PatchEntry<decltype(+function), u32>{
        .level = level,
        .type = WWFC_PATCH_TYPE_BRANCH_CTR,
        .address = address,
        .arg0 = +function,
        .arg1 = tempReg,
    };
}

constexpr auto
CallWithCTR(u8 level, u32 address, auto function, u32 tempReg = 12)
{
    return PatchEntry<decltype(+function), u32>{
        .level = level,
        .type = WWFC_PATCH_TYPE_BRANCH_CTR_LINK,
        .address = address,
        .arg0 = +function,
        .arg1 = tempReg,
    };
}

#define _WWFC_DEFINE_PATCH2(NUM)                                               \
    __attribute__((                                                            \
        __section__(".wwfc_patch")                                             \
    )) constinit auto __wwfc_patch_##NUM

#define _WWFC_DEFINE_PATCH1(NUM) _WWFC_DEFINE_PATCH2(NUM)
#define WWFC_DEFINE_PATCH _WWFC_DEFINE_PATCH1(__COUNTER__)

// Silly hack to load this address through a fixup address
#define _WWFC_DEFINE_CTR_STUB2(_ADDRESS, _PROTOTYPE, _COUNTER, ...)            \
    extern int _CTR_STUB_DEST_##_COUNTER AT(_ADDRESS);                         \
    extern "C" {                                                               \
    constinit int* _CTR_STUB_##_COUNTER = &_CTR_STUB_DEST_##_COUNTER;          \
    }                                                                          \
                                                                               \
    __attribute__((__weak__)) _PROTOTYPE                                       \
    {                                                                          \
        asm volatile("" #__VA_ARGS__ "\n"                                      \
                     "bl 4\n"                                                  \
                     "mflr 12\n"                                               \
                     "lwz 12, _CTR_STUB_" #_COUNTER " - (. - 4)(12)\n"         \
                     "mtctr 12\n"                                              \
                     "bctr\n"                                                  \
                     :);                                                       \
        __builtin_unreachable();                                               \
    }

#define _WWFC_DEFINE_CTR_STUB_SAVE_LR2(_ADDRESS, _PROTOTYPE, _COUNTER, ...)    \
    extern int _CTR_STUB_DEST_##_COUNTER AT(_ADDRESS);                         \
    extern "C" {                                                               \
    constinit int* _CTR_STUB_##_COUNTER = &_CTR_STUB_DEST_##_COUNTER;          \
    }                                                                          \
                                                                               \
    __attribute__((__weak__)) _PROTOTYPE                                       \
    {                                                                          \
        asm volatile("" #__VA_ARGS__ "\n"                                      \
                     "mflr 11\n"                                               \
                     "bl 4\n"                                                  \
                     "mflr 12\n"                                               \
                     "mtlr 11\n"                                               \
                     "lwz 12, _CTR_STUB_" #_COUNTER " - (. - 8)(12)\n"         \
                     "mtctr 12\n"                                              \
                     "bctr\n"                                                  \
                     :);                                                       \
        __builtin_unreachable();                                               \
    }

#define _WWFC_DEFINE_CTR_STUB1(_ADDRESS, _PROTOTYPE, NUM, ...)                 \
    _WWFC_DEFINE_CTR_STUB2(_ADDRESS, _PROTOTYPE, NUM, __VA_ARGS__)
#define WWFC_DEFINE_CTR_STUB(_ADDRESS, _PROTOTYPE, ...)                        \
    _WWFC_DEFINE_CTR_STUB1(_ADDRESS, _PROTOTYPE, __COUNTER__, __VA_ARGS__)

#define _WWFC_DEFINE_CTR_STUB_SAVE_LR1(_ADDRESS, _PROTOTYPE, NUM, ...)         \
    _WWFC_DEFINE_CTR_STUB_SAVE_LR2(_ADDRESS, _PROTOTYPE, NUM, __VA_ARGS__)
#define WWFC_DEFINE_CTR_STUB_SAVE_LR(_ADDRESS, _PROTOTYPE, ...)                \
    _WWFC_DEFINE_CTR_STUB_SAVE_LR1(                                            \
        _ADDRESS, _PROTOTYPE, __COUNTER__, __VA_ARGS__                         \
    )

} // namespace wwfc::Patch

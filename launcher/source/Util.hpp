#pragma once

#include <gctypes.h>
#include <gcutil.h>

template <typename T>
constexpr T AlignUp(T num, unsigned int align)
{
    u32 raw = (u32) num;
    return (T) ((raw + align - 1) & -align);
}

template <typename T>
constexpr T AlignDown(T num, unsigned int align)
{
    u32 raw = (u32) num;
    return (T) (raw & -align);
}

template <class T>
constexpr bool IsAligned(T addr, unsigned int align)
{
    return !((u32) addr & (align - 1));
}

template <class T1, class T2>
constexpr bool CheckBounds(T1 bounds, u32 boundLen, T2 buffer, u32 len)
{
    u32 low = (u32) bounds;
    u32 high = low + boundLen;
    u32 inside = (u32) buffer;
    u32 insidehi = inside + len;

    return (high >= low) && (insidehi >= inside) && (inside >= low) &&
           (insidehi <= high);
}

template <class T>
constexpr bool InMEM1(T addr)
{
    const u32 value = (u32) addr;
    return value < 0x01800000;
}

template <class T>
constexpr bool InMEM2(T addr)
{
    const u32 value = (u32) addr;
    return (value >= 0x10000000) && (value < 0x14000000);
}

template <class T>
constexpr bool InMEM1Effective(T addr)
{
    const u32 value = (u32) addr;
    return (value >= 0x80000000) && (value < 0x81800000);
}

template <class T>
constexpr bool InMEM2Effective(T addr)
{
    const u32 value = (u32) addr;
    return (value >= 0x90000000) && (value < 0x94000000);
}

constexpr u32 U64Hi(u64 value)
{
    return value >> 32;
}

constexpr u32 U64Lo(u64 value)
{
    return value & 0xFFFFFFFF;
}

static inline u32 ReadU8(u32 address)
{
    return *(volatile u8*) address;
}

static inline u32 ReadU16(u32 address)
{
    return *(volatile u16*) address;
}

static inline u32 ReadU32(u32 address)
{
    return *(volatile u32*) address;
}

static inline void WriteU8(u32 address, u8 value)
{
    *(volatile u8*) address = value;
}

static inline void WriteU16(u32 address, u16 value)
{
    *(volatile u16*) address = value;
}

static inline void WriteU32(u32 address, u32 value)
{
    *(volatile u32*) address = value;
}

static inline void MaskU8(u32 address, u8 clear, u8 set)
{
    *(volatile u8*) address = ((*(volatile u8*) address) & ~clear) | set;
}

static inline void MaskU16(u32 address, u16 clear, u16 set)
{
    *(volatile u16*) address = ((*(volatile u16*) address) & ~clear) | set;
}

static inline void MaskU32(u32 address, u32 clear, u32 set)
{
    *(volatile u32*) address = ((*(volatile u32*) address) & ~clear) | set;
}

constexpr u32 MakeBranch(u32 from, u32 to)
{
    return 0x48000000 | ((to - from) & 0x03FFFFFC);
}
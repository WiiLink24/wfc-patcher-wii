#include "IOS.hpp"
#include "Util.hpp"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <gctypes.h>
#include <ogc/cache.h>
#include <ogc/lwp.h>

static bool IsDolphinImpl()
{
    // Newer versions of Dolphin have an IOS device called /dev/dolphin
    s32 fd = IOS_Open("/dev/dolphin", 0);
    if (fd >= 0) {
        IOS_Close(fd);
        return true;
    }

    // Older versions do not have this device, but they can be detected by the
    // lack of /dev/sha
    fd = IOS_Open("/dev/sha", 0);
    if (fd >= 0) {
        IOS_Close(fd);
        return false;
    }

    return fd == -6; // ENOENT
}

/**
 * Returns true if the program is running on Dolphin.
 */
bool IOS::IsDolphin()
{
    static bool isDolphin = IsDolphinImpl();
    return isDolphin;
}

/**
 * This is the stub THUMB code to be copied to the beginning of MEM1 (should
 * be max 32 bytes).
 */
static const u32 ARMStage0[8] = {
    // .thumb
    // @ Immediately switch to ARM mode. This instruction would in theory be
    // @ ignored if in ARM mode anyway, due to it requiring the N==1
    // condition.
    // bx      pc
    // nop
    0x477846C0, //
    // .arm
    0xE59F1010, // ldr     r1, =ENTRY_POINT
    0xEE071F36, // mcr     p15, 0, r1, c7, c6, 1 @ Invalidate DCache
    0xE3A00000, // mov     r0, #0
    0xEE070F9A, // mcr     p15, 0, r0, c7, c10, 4 @ Drain write buffer
    0xEE071F35, // mcr     p15, 0, r1, c7, c5, 1 @ Invalidate ICache
    0xE12FFF11, // bx      r1
    0x00000002, // .long   ENTRY_POINT @ Set on runtime
};

static u32 ARMStage1[] alignas(32) = {
    0xE59F2014, // ldr     r2, =STAGE1_END
                // L_CacheLoop:
    0xEE071F36, // mcr     p15, 0, r1, c7, c6, 1 @ Invalidate DCache
    0xEE071F35, // mcr     p15, 0, r1, c7, c5, 1 @ Invalidate ICache
    0xE2811020, // add     r1, r1, #32
    0xE1510002, // cmp     r1, r2
    0xBAFFFFFA, // blt     L_CacheLoop
    0xEA000001, // b       Entry
    0x00000000, // STAGE1_END @ Set on runtime
    0x00000000, // MESSAGE_VALUE
    // Entry:
    // @ Flush the entire data cache
    // L_FlushLoop:
    0xEE17FF7A, // mrc     p15, 0, pc, c7, c10, 3
    0x1AFFFFFD, // bne     L_FlushLoop
    // @ Give PPC full bus access
    0xE3A02536, // mov     r2, #0x0D800000
    0xE5921060, // ldr     r1, [r2, #0x60]
    0xE3811008, // orr     r1, #0x08
    0xE5821060, // str     r1, [r2, #0x60]
    0xE5921064, // ldr     r1, [r2, #0x64]
    0xE381113A, // orr     r1, #0x8000000E
    0xE3811EDF, // orr     r1, #0x00000DF0
    0xE5821064, // str     r1, [r2, #0x64]
    // @ Wait for PPC to finish patching
    0xE24F3034, // adr     r3, MESSAGE_VALUE
    // L_PPCWaitLoop:
    0xEE073F36, // mcr     p15, 0, r3, c7, c6, 1 @ Invalidate DCache
    0xE5932000, // ldr     r2, [r3]
    0xE3520001, // cmp     r2, #1
    0x1AFFFFFB, // bne     L_PPCWaitLoop
    // @ Invalidate the entire ICache
    0xEE070F15, // mcr     p15, 0, r0, c7, c5, 0
    // @ Flush the entire data cache (again)
    // L_FlushLoop2:
    0xEE17FF7A, // mrc     p15, 0, pc, c7, c10, 3
    0x1AFFFFFD, // bne     L_FlushLoop2
    // @ Send acknowledge back to PPC
    0xE3A00002, // mov     r0, #2
    0xE5830000, // str     r0, [r3]
    0xEE073F3A, // mcr     p15, 0, r3, c7, c10, 1 @ Flush DCache
    // @ Set NL=0 to skip the first instruction in case the exploit runs
    // again
    0xE3A00000, // mov     r0, #0
    0xE3500000, // cmp     r0, #0
    // @ Overwrite the reserved handler to infinitely loop
    0xE59F1004, // ldr     r1, =0xFFFF0014
    0xE5811020, // str     r1, [r1, #0x20]
    0xE12FFF11, // bx      r1
    // Data pool;
    0xFFFF0014,
};

template <typename T>
static void WriteVU32(T addr, u32 value)
{
    *reinterpret_cast<volatile u32*>(u32(addr) | 0xC0000000) = value;
}

template <typename T>
static u32 ReadVU32(T addr)
{
    return *reinterpret_cast<volatile u32*>(u32(addr) | 0xC0000000);
}

template <typename T>
static void MaskVU32(T addr, u32 clear, u32 set)
{
    u32 temp = ReadVU32(addr);
    temp &= ~clear;
    temp |= set;
    WriteVU32(addr, temp);
}

template <u32 N>
static bool Compare(const u32 (&mask)[N], const u32 (&pattern)[N], u32 addr)
{
    for (u32 i = 0; i < N; i++) {
        if ((ReadVU32(addr + i * 4) & mask[i]) != pattern[i]) {
            return false;
        }
    }
    return true;
}

template <u32 N>
static u32
Find(const u32 (&mask)[N], const u32 (&pattern)[N], u32 start, u32 end)
{
    for (u32 addr = start; addr <= end - N * 4; addr += 4) {
        if (Compare(mask, pattern, addr)) {
            return addr;
        }
    }
    return 0;
}

template <u32 N>
static void Copy(const u32 (&mask)[N], const u32 (&pattern)[N], u32 addr)
{
    for (u32 i = 0; i < N; i++) {
        MaskVU32(addr + i * 4, mask[i], pattern[i] & mask[i]);
    }
}

/**
 * Implementation of the /dev/sha Init exploit, discovered by Palapeli in 2021.
 */
static bool SendExploit()
{
    // No need to run the exploit on Dolphin
    if (IOS::IsDolphin()) {
        return true;
    }

    // Open /dev/sha
    s32 fd = IOS_Open("/dev/sha", 0);
    if (fd < 0) {
        std::fprintf(stderr, "SendExploit: IOS_Open failed: %d\n", fd);
        return false;
    }

    // Setup ARM code

    u32* mem1 = reinterpret_cast<u32*>(0x80000000);
    std::memcpy(mem1, ARMStage0, sizeof(ARMStage0));
    mem1[7] = reinterpret_cast<u32>(ARMStage1) - 0x80000000;

    ARMStage1[7] =
        (reinterpret_cast<u32>(ARMStage1) - 0x80000000) + sizeof(ARMStage1);
    DCFlushRange(&ARMStage1[0], 32);

    IOS::IOVector vec[4] alignas(32) = {};
    vec[0].data = NULL;
    vec[0].len = 0;

    // Due to the length being 0, the memory bounds check performed by IOS will
    // not fail here. Without checking the length, the SHA-1 Init call will
    // write the following words to the destination:
    //
    // 00: 67452301
    // 04: EFCDAB89
    // 08: 98BADCFE
    // 0C: 10325476
    // 10: C3D2E1F0
    // 14: 00000000
    // 18: 00000000
    //
    // The zero words are useful here, because due to a flaw in the design of
    // IOS, 0 and NULL always point to the beginning of MEM1, an area controlled
    // by PPC. Most exploits utilize this by overwriting the LR on the stack to
    // jump to the beginning of MEM1, however we can go a step further...
    //
    // For whatever reason the thread that handles /dev/sha runs in ARM system
    // mode. This means that all kernel-only or read-only memory is now mapped
    // as read/write. We can use this to instead attack the context of the idle
    // thread, which is always located at 0xFFFE0000 in every version of IOS and
    // also runs in system mode, making our exploit much more stable.
    //
    // We want to write one of the 0 words to the stored PC at 0xFFFE0040, so
    // here we subtract that by 0x18, trashing various other registers that
    // don't matter in the process.
    vec[1].data = reinterpret_cast<void*>(0xFFFE0028);
    vec[1].len = 0;

    // This vector is unused by this specific call, so we will utilize it for
    // invalidating the dcache on the IOS side. Cache safety is important here;
    // see the CTGP-R Channel installer freezing due to a battle with the cache.
    vec[2].data = mem1;
    vec[2].len = 32;

    // Make the exploit call
    s32 ret = IOS_Ioctlv(fd, 0, 1, 2, vec);

    // Close /dev/sha
    IOS_Close(fd);

    // IOS_Ioctlv should never fail if the exploit succeeded
    if (ret != 0) {
        // Send finished message to ARM
        WriteVU32(reinterpret_cast<u32>(&ARMStage1[8]), 1);

        std::fprintf(stderr, "SendExploit: IOS_Ioctlv failed: %d\n", ret);
        return false;
    }

    // Wait for SRNPROT to update
    auto startTime = std::time(nullptr);
    while (ReadVU32(0x0D4F0000) != 0xE59FF018) {
        LWP_YieldThread();

        if (std::time(nullptr) - startTime >= 1) {
            std::fprintf(
                stderr, "SendExploit: Timeout waiting for SRNPROT to update\n"
            );
            return false;
        }
    }

    return true;
}

static void CloseExploit(bool error)
{
    // Remove bus access
    MaskVU32(0x0D800060, 0x08, 0);
    MaskVU32(0x0D800064, 0x00000DFE, 0);
    MaskVU32(0x0D800064, 0x80000000, 0);

    // Send finished message to ARM
    WriteVU32(ARMStage1 + 8, 1);

    if (error) {
        return;
    }

    // Wait for response from ARM
    auto startTime = std::time(nullptr);
    while (ReadVU32(ARMStage1 + 8) != 2) {
        LWP_YieldThread();

        if (std::time(nullptr) - startTime >= 1) {
            std::fprintf(
                stderr, "CloseExploit: Timeout waiting for response from ARM\n"
            );
            std::abort();
        }
    }
}

/**
 * Check if the address is a valid jumptable pointer.
 */
bool ValidJumptablePtr(u32 address)
{
    return address >= 0xFFFF0040 && !(address & 3);
}

/**
 * Check if the address is a valid kernel code pointer.
 */
bool ValidKernelCodePtr(u32 address)
{
    return address >= 0xFFFF0040 && (address & 2) != 2;
}

/**
 * Find the syscall table in memory. Returns SRAM mirror address (0xFFFxxxxx).
 */
static u32 FindSyscallTable()
{
    u32 undefinedHandler = ReadVU32(0x0D4F0024);
    if (ReadVU32(0x0D4F0004) != 0xE59FF018 || undefinedHandler < 0xFFFF0040 ||
        undefinedHandler >= 0xFFFFF000 || (undefinedHandler & 3) ||
        ReadVU32(undefinedHandler - 0xF2B00000) != 0xE9CD7FFF) {
        // Invalid undefined handler
        return 0;
    }

    for (s32 i = 0x300; i < 0x700; i += 4) {
        if (ReadVU32(undefinedHandler - 0xF2B00000 + i) == 0xE6000010 &&
            ValidJumptablePtr(ReadVU32(undefinedHandler - 0xF2B00000 + i + 4)
            ) &&
            ValidJumptablePtr(ReadVU32(undefinedHandler - 0xF2B00000 + i + 8)
            )) {
            return ReadVU32(undefinedHandler - 0xF2B00000 + i + 8);
        }
    }

    // No jumptable found
    return 0;
}

/**
 * Perform an IOS exploit and do the specified patches.
 */
void IOS::PatchIOS(u32 patchFlags)
{
    // Remove bus access in case we already have it
    MaskVU32(0x0D800060, 0x08, 0);
    MaskVU32(0x0D800064, 0x00000DFE, 0);
    MaskVU32(0x0D800064, 0x80000000, 0);

    // Send the exploit
    if (!SendExploit()) {
        CloseExploit(true);
        std::abort();
        return;
    }

    std::printf("PatchIOS: Exploit succeeded\n");

    // Patch RSA signature verification
    if (patchFlags & PATCH_IOSC_RSA_VERIFY) {

        static const u16 NewVerifySign[] alignas(32) = {
            0x2000, // mov     r0, #0
            0x4770, // bx      lr
        };

        u32 syscallTab = FindSyscallTable();
        if (syscallTab == 0) {
            std::fprintf(stderr, "PatchIOS: FindSyscallTable failed\n");
            CloseExploit(true);
            std::abort();
            return;
        }

        // TODO: This might be different between IOS versions
        WriteVU32(
            (syscallTab + 0x6C * 4) - 0xF2B00000,
            (reinterpret_cast<u32>(NewVerifySign) - 0x80000000) | 1
        );
    }

    // Import Korean common key
    if (patchFlags & PATCH_IOSC_KOREAN_KEY) {
        // Find the common key in the Starlet SRAM
        u32 keyMask[] = {0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
                         0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
                         0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF};
        u32 keyPattern[] = {0x01EBE42A, 0x225E8593, 0xE448D9C5,
                            0x457381AA, 0xF7000000, 0x00000000,
                            0x00000000, 0x00000000, 0x00000000};
        u32 start = Find(keyMask, keyPattern, 0x0D410000, 0x0D420000);
        if (start) {
            // The new common key should be at a fixed offset. For Korean Wiis,
            // it's already there. For other Wiis, it's all zeros instead
            start += 0x6C;

            u32 noKeyMask[] = {0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
                               0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
                               0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF};
            u32 noKeyPattern[] = {0x01000000, 0x00000000, 0x00000000,
                                  0x00000000, 0x00000000, 0x00000000,
                                  0x00000000, 0x00000000, 0x00000000};
            if (Compare(noKeyMask, noKeyPattern, start)) {
                // Copy the new key
                u32 newKeyMask[] = {0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
                                    0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
                                    0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF};
                u32 newKeyPattern[] = {0x0163B82B, 0xB4F4614E, 0x2E13F2FE,
                                       0xFBBA4C9B, 0x7E000000, 0x00000000,
                                       0x00000000, 0x00000000, 0x00000000};
                Copy(newKeyMask, newKeyPattern, start);
            }
        }
    }

    CloseExploit(false);
    std::printf("PatchIOS: Patch succeeded\n");
}

static lwp_t s_patchThread;
static bool s_patchStarted = false;

static void* PatchIOSThread(void* arg)
{
    u32 patchFlags = reinterpret_cast<u32>(arg);
    IOS::PatchIOS(patchFlags);
    return nullptr;
}

void IOS::PatchIOSAsync(u32 patchFlags)
{
    // Start a new thread to perform the patch
    s32 ret = LWP_CreateThread(
        &s_patchThread, PatchIOSThread, reinterpret_cast<void*>(patchFlags),
        NULL, 0, 0x80
    );
    if (ret != 0) {
        std::fprintf(
            stderr, "PatchIOSAsync: LWP_CreateThread failed: %d\n", ret
        );
        std::abort();
    }

    s_patchStarted = true;
}

void IOS::WaitForPatchIOS()
{
    if (!s_patchStarted) {
        return;
    }

    // Wait for the patch thread to finish
    s32 ret = LWP_JoinThread(s_patchThread, nullptr);
    if (ret != 0) {
        std::fprintf(
            stderr, "WaitForPatchIOS: LWP_JoinThread failed: %d\n", ret
        );
        std::abort();
    }
}
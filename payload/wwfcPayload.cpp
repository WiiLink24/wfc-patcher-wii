#include "import/eggException.hpp"
#include "wwfcLogin.hpp"
#include "wwfcPatch.hpp"
#include "wwfcSupport.hpp"
#include "wwfcUtil.h"
#include <wwfcCommon.h>
#include <wwfcError.h>

namespace wwfc::Payload
{

// Define low level symbols

/**
 * Payload entry point. Applies global offset table and fixup relocations, and
 * then calls wwfc_payload_entry_no_got.
 */
s32 Entry(wwfc_payload* payload) asm("wwfc_payload_entry");

/**
 * Payload entry point. Does not apply global offset table and fixup
 * relocations. Automatically called by wwfc_payload_entry.
 */
s32 EntryAfterGOT(wwfc_payload* payload) asm("wwfc_payload_entry_no_got");

// Symbols provided in the linker script
extern u32 GOTStart asm("_G_GOTStart");
extern u32 GOTEnd asm("_G_GOTEnd");
extern u32 FixupStart asm("_G_FixupStart");
extern u32 FixupEnd asm("_G_FixupEnd");
extern wwfc_patch PatchStart asm("_G_WWFCPatchStart");
extern wwfc_patch PatchEnd asm("_G_WWFCPatchEnd");
extern u32 CTORSStart asm("__CTORS_START__");
extern u32 CTORSEnd asm("__CTORS_END__");
extern u8 PayloadEnd asm("_G_End");

SECTION("wwfc_header")
const wwfc_payload Header = {
    .header =
        {
            .magic =
                {'W', 'W', 'F', 'C', '/', 'P', 'a', 'y', 'l', 'o', 'a', 'd'},
            .total_size = u32(&PayloadEnd),
            .signature = {},
        },
    .salt = {},
    .info =
        {
            .format_version = 1,
            .format_version_compat = 1,
            .name = PAYLOAD_NAME,
            .version = 4,
            .got_start = u32(&GOTStart),
            .got_end = u32(&GOTEnd),
            .fixup_start = u32(&FixupStart),
            .fixup_end = u32(&FixupEnd),
            .patch_list_offset = u32(&PatchStart),
            .patch_list_end = u32(&PatchEnd),
            .entry_point = u32(&Entry),
            .entry_point_no_got = u32(&EntryAfterGOT),
            .reserved = {},
            .build_timestamp = __TIMESTAMP__,
        },
};

/**
 * Payload entry point. Applies global offset table and fixup relocations, and
 * then calls wwfc_payload_entry_no_got.
 */
ASM_FUNCTION( //
    s32 Entry(wwfc_payload* payload),
    // clang-format off
    addi    r6, r3, (_G_GOTStart - 4)@l;
    addi    r7, r3, (_G_GOTEnd - 4)@l;
    addi    r8, r3, (_G_FixupEnd - 4)@l;
L_LoopStart:;
    cmplw   r6, r7;
    bge-    L_LoopFixupStart;
    lwzu    r9, 0x4(r6);
    rlwinm. r0, r9, 0, 0x80000000;
    bne-    L_LoopStart;
    add     r9, r9, r3;
    stw     r9, 0(r6);
    b       L_LoopStart;
L_LoopFixupStart:;
    cmplw   r6, r8;
    bge-    L_LoopFixupEnd;
    lwzu    r9, 0x4(r6);
    lwzx    r10, r9, r3;
    rlwinm. r0, r10, 0, 0x80000000;
    bne-    L_LoopFixupStart;
    add     r10, r10, r3;
    stwx    r10, r9, r3;
    b       L_LoopFixupStart;
L_LoopFixupEnd:;
    b       wwfc_payload_entry_no_got;
    // clang-format on
)

extern struct LoMem {
    u32 discId;
    u16 makerCode;
    u8 discNumber;
    u8 discVersion;
} g_LoMem AT(0x80000000);

static void CallCtors(const wwfc_payload* const payload)
{
    void (*ctor)();

    for (const u32* ctorAddress = &CTORSStart; ctorAddress < &CTORSEnd;
         ++ctorAddress) {
        u32 ctorOffset = *ctorAddress;

        if (ctorOffset == 0x00000000 || ctorOffset == 0xFFFFFFFF) {
            continue;
        }

        ctor = (decltype(ctor)
        ) (reinterpret_cast<const char*>(payload) + ctorOffset);
        (*ctor)();
    }
}

/**
 * Payload entry point. Does not apply global offset table and fixup
 * relocations. Automatically called by wwfc_payload_entry.
 */
s32 EntryAfterGOT(wwfc_payload* payload)
{
#if TITLE_TYPE == TITLE_TYPE_DISC
    // Verify that the current game is the one this payload is built for
    if (g_LoMem.discId != GAME_ID) {
        return WL_ERROR_PAYLOAD_GAME_MISMATCH;
    }

    // Check that the disc version matches
    // Don't check for Brawl US or JP because they are identical except for one
    // byte
#  if !RSBED01 && !RSBED02 && !RSBJD00 && !RSBJD01
    if (g_LoMem.discVersion != TITLE_VERSION) {
        return WL_ERROR_PAYLOAD_GAME_MISMATCH;
    }
#  endif
#endif

    EGG::Exception::SetUserCallBack(nullptr);

    CallCtors(payload);

    Patch::ApplyPatchList(
        reinterpret_cast<u32>(payload), &PatchStart,
        std::distance(&PatchStart, &PatchEnd)
    );

    Support::ChangeAuthURL();
    Login::Init();

    return WL_ERROR_PAYLOAD_OK;
}

} // namespace wwfc::Payload

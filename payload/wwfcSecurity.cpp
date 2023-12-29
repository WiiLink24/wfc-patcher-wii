#include "import/mkwNet.hpp"
#include "wwfcPatch.hpp"
#include "wwfcUtil.h"
#include <cstddef>

namespace wwfc::Security
{

WWFC_DEFINE_PATCH = {
#if ADDRESS_DWCi_GetGPBuddyAdditionalMsg
    // SERVER TO CLIENT VULNERABILITY
    // CVE-ID: CVE-2023-45887
    // https://cve.mitre.org/cgi-bin/cvename.cgi?name=CVE-2023-45887
    Patch::CallWithCTR(
        WWFC_PATCH_LEVEL_CRITICAL, //
        ADDRESS_DWCi_GetGPBuddyAdditionalMsg + 0x90, //
        ASM_LAMBDA(
            // clang-format off
            cmplwi    r31, 0xF; // The destination buffer is of size 0x10; leave room for the null terminator
            mflr      r9;
            mr        r5, r31;
            mr        r4, r28;
            mr        r3, r27;
            ble+      L_ValidLength;
            addi      r9, r9, -0x1C; // return -1
            mtctr     r9;
            bctr;

        L_ValidLength:
            b         memcpy;
            // clang-format on
        )
    ),
#endif
};

// SERVER|CLIENT TO CLIENT VULNERABILITY
// Match command stack buffer overflow. This exists in nearly every Wii game.
WWFC_DEFINE_PATCH = {
#if ADDRESS_PATCH_SECURITY_GT2MATCHCOMMAND
    // CLIENT TO CLIENT VULNERABILITY
    // The peer to peer match command exploit
    Patch::CallWithCTR(
        WWFC_PATCH_LEVEL_CRITICAL, //
        ADDRESS_PATCH_SECURITY_GT2MATCHCOMMAND - 0x24, // 0x800E591C
        ASM_LAMBDA(
            // clang-format off
            lbz     r5, 0x11(sp);
            addi    r0, r5, 0x14;
            cmplw   r31, r0;
            bnelr-; // Error "Got wrong data size GT2 command."

            // Check the maximum length to prevent a buffer overflow
            cmplwi  r5, 0x80;
            bgtlr-; // Error "Got wrong data size GT2 command."

            // OK, jump to the copy routine
            mflr    r12;
            addi    r12, r12, 0x14;
            mtctr   r12;
            bctr;
            // clang-format on
        )
    ),
#endif

#if ADDRESS_PATCH_SECURITY_QR2MATCHCOMMAND
    // SERVER TO CLIENT VULNERABILITY
    // The QR2/MASTER server-sent match command exploit
    Patch::CallWithCTR(
        WWFC_PATCH_LEVEL_CRITICAL, //
        ADDRESS_PATCH_SECURITY_QR2MATCHCOMMAND, // 0x800E5AC8
        ASM_LAMBDA(
            // clang-format off
            lbz     r5, 0x11(sp);
            // Check the maximum length to prevent a buffer overflow
            cmplwi  r5, 0x80;
            bgt-    L_SBCommandError;

            // OK, copy the data to the stack
            addi    r3, r1, 0x1C;
            addi    r4, r28, 0x14;
            // Call and return
            b       memcpy@local;

        L_SBCommandError:;
            // Jump to "Got different version SBcommand." error
            mflr    r12;
            subi    r12, r12, 0x24;
            mtctr   r12;
            bctr;
            // clang-format on
        )
    ),
#endif
};

#if RMC || RMCN

// SERVER TO CLIENT VULNERABILITY
// Patch for Mario Kart Wii friend status (locstring) stack buffer overflow.
// Located in DWC_GetFriendStatusData, this one is a bit annoying because it
// could exist in other games, it just depends on the size the caller is
// expecting.
WWFC_DEFINE_PATCH = {
    Patch::WriteASM(
        WWFC_PATCH_LEVEL_CRITICAL, //
        RMCX_PORT(
            0x800CE220, 0x800CE180, 0x800CE140, 0x800CE280, // Disc
            0x800B5D98, 0x800B5D08, 0x800B5CE8, 0x800B5E08 // Channel
        ),
        1, ASM_LAMBDA(li r6, 0x10)
    ),
};

#endif

#if RMC

static bool IsValidRACEPacket(mkw::Net::RACEPacket* packet, u32 packetSize)
{
    using namespace mkw::Net;

    if (packetSize < sizeof(RACEPacket)) {
        return false;
    }

    extern u32 packetBufferSizes[sizeof(RACEPacket::sizes)] AT(
        RMCXD_PORT(0x8089A194, 0x80895AC4, 0x808992F4, 0x808885CC)
    );

    // Check packet sizes against the actual buffer sizes, allowing game mods to
    // expand them

    u8 headerSize = packet->sizes[RACEPacket::HEADER];
    if (headerSize < 0x10 ||
        headerSize > packetBufferSizes[RACEPacket::HEADER]) {
        return false;
    }

    u32 expectedPacketSize = headerSize;

    u8 raceHeader1Size = packet->sizes[RACEPacket::RACEHEADER_1];
    if (raceHeader1Size != 0 &&
        (raceHeader1Size < 0x28 ||
         raceHeader1Size > packetBufferSizes[RACEPacket::RACEHEADER_1])) {
        return false;
    }

    expectedPacketSize += raceHeader1Size;

    u8 raceHeader2Size = packet->sizes[RACEPacket::RACEHEADER_2];
    if (raceHeader2Size != 0 &&
        (raceHeader2Size < 0x28 ||
         raceHeader2Size > packetBufferSizes[RACEPacket::RACEHEADER_2])) {
        return false;
    }

    expectedPacketSize += raceHeader2Size;

    u8 roomSelectSize = packet->sizes[RACEPacket::ROOM_SELECT];
    if (roomSelectSize != 0 &&
        (roomSelectSize < 0x04 ||
         roomSelectSize > packetBufferSizes[RACEPacket::ROOM_SELECT])) {
        return false;
    }

    expectedPacketSize += roomSelectSize;

    u8 raceDataSize = packet->sizes[RACEPacket::RACEDATA];
    if (raceDataSize != 0 &&
        (raceDataSize < 0x40 ||
         raceDataSize > packetBufferSizes[RACEPacket::RACEDATA] ||
         raceDataSize % 0x40)) {
        return false;
    }

    expectedPacketSize += raceDataSize;

    u8 userSize = packet->sizes[RACEPacket::USER];
    if (userSize != 0) {
        if (userSize < 0xC0 || userSize > packetBufferSizes[RACEPacket::USER]) {
            return false;
        }

        // Mii count must be 2
        auto userPacket = reinterpret_cast<mkw::Net::USERPacket*>(
            u32(packet) + expectedPacketSize
        );
        if (userPacket->miiGroupCount != 2) {
            return false;
        }
    }

    expectedPacketSize += userSize;

    u8 itemSize = packet->sizes[RACEPacket::ITEM];
    if (itemSize != 0 &&
        (itemSize < 0x08 || itemSize > packetBufferSizes[RACEPacket::ITEM] ||
         itemSize % 0x08)) {
        return false;
    }

    expectedPacketSize += itemSize;

    // SECURITY TODO: There is some kind of (harmless?) overflow in EVENT,
    // not patched by Wiimmfi or CTGP
    u8 eventSize = packet->sizes[RACEPacket::EVENT];
    if (eventSize != 0 && (eventSize < 0x18 ||
                           eventSize > packetBufferSizes[RACEPacket::EVENT])) {
        return false;
    }

    expectedPacketSize += eventSize;

    return packetSize >= expectedPacketSize;
}

// CLIENT TO CLIENT VULNERABILITY
// Patch for Mario Kart Wii RACE exploit. This was the first RCE exploit
// discovered in a Wii game. Originally discovered by XeR, but then rediscovered
// by Star, who reported the exploit and then released it.
// CVE-ID: CVE-2023-35856
// https://cve.mitre.org/cgi-bin/cvename.cgi?name=CVE-2023-35856
WWFC_DEFINE_PATCH = {Patch::BranchWithCTR( //
    WWFC_PATCH_LEVEL_CRITICAL, //
    RMCXD_PORT(0x80658604, 0x8065417C, 0x80657C70, 0x8064691C), //
    [](mkw::Net::RKNetController* rkNetController,
       mkw::Net::RACEPacket* racePacket, u32 packetSize, u32 _,
       u8 playerAid) -> void {
        if (!IsValidRACEPacket(racePacket, packetSize)) {
            LONGCALL int DWC_CloseConnectionHard(u8 playerAid)
                AT(RMCXD_PORT(0x800D2000, 0x800D1F60, 0x800D1F20, 0x800D2060));
            DWC_CloseConnectionHard(playerAid);

            return;
        }

        rkNetController->processRACEPacket(playerAid, racePacket, packetSize);
    }
)};

#endif

} // namespace wwfc::Security

#include "import/mkwNet.hpp"
#include "wwfcPatch.hpp"
#include "wwfcUtil.h"
#include <cstddef>

namespace wwfc::Security
{

// SERVER|CLIENT TO CLIENT VULNERABILITY
// Match command stack buffer overflow. This exists in nearly every Wii game.
// SECURITY TODO: Patch the server to client one in GPCM that only exists
// in DS games and early Wii games (such as Super Smash Bros. Brawl). This will
// allow us to lift the length restriction on the server.

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

    for (u32 expectedPacketSize = 0, i = 0; i < 8; i++) {
        if (expectedPacketSize + packet->sizes[i] > packetSize) {
            return false;
        }

        // Not a better place to check this, the Mii count in USER
        if (i == RACEPacket::USER && packet->sizes[i] != 0) {
            if (packet->sizes[i] != 0xC0 ||
                *(u16*) (u32(packet) + expectedPacketSize + 0x4) != 2) {
                return false;
            }
        }

        expectedPacketSize += packet->sizes[i];
    }

    // TODO: Check against the actual buffer size to allow mods to expand it

    if (packet->sizes[RACEPacket::HEADER] != 0x10) {
        return false;
    }

    if (packet->sizes[RACEPacket::RACEHEADER_1] != 0 &&
        packet->sizes[RACEPacket::RACEHEADER_1] != 0x28) {
        return false;
    }

    if (packet->sizes[RACEPacket::RACEHEADER_2] != 0 &&
        packet->sizes[RACEPacket::RACEHEADER_2] != 0x28) {
        return false;
    }

    if (packet->sizes[RACEPacket::ROOM_SELECT] != 0 &&
        packet->sizes[RACEPacket::ROOM_SELECT] != 0x4 &&
        packet->sizes[RACEPacket::ROOM_SELECT] != 0x38) {
        return false;
    }

    if (packet->sizes[RACEPacket::RACEDATA] != 0 &&
        packet->sizes[RACEPacket::RACEDATA] != 0x40 &&
        packet->sizes[RACEPacket::RACEDATA] != 0x80) {
        return false;
    }

    // The size of the 'USER' portion of the packet was previously validated

    if (packet->sizes[RACEPacket::ITEM] != 0 &&
        packet->sizes[RACEPacket::ITEM] != 0x8 &&
        packet->sizes[RACEPacket::ITEM] != 0x10) {
        return false;
    }

    // SECURITY TODO: There is some kind of (harmless?) overflow in EVENT,
    // not patched by Wiimmfi or CTGP
    if (packet->sizes[RACEPacket::EVENT] != 0 &&
        (packet->sizes[RACEPacket::EVENT] < 0x18 ||
         packet->sizes[RACEPacket::EVENT] >= 0xF9)) {
        return false;
    }

    return true;
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

    [](void* rkNetController, mkw::Net::RACEPacket* packet, u32 packetSize,
       u32 _, u8 playerAid) -> void {
        if (!IsValidRACEPacket(packet, packetSize)) {
            LONGCALL int DWC_CloseConnectionHard(u8 playerAid)
                AT(RMCXD_PORT(0x800D2000, 0x800D1F60, 0x800D1F20, 0x800D2060));

            DWC_CloseConnectionHard(playerAid);

            return;
        }

        LONGCALL void RKNetController_ProcessRACEPacket(
            void* rkNetController, u8 playerAid, mkw::Net::RACEPacket* packet,
            u32 packetSize
        ) AT(RMCXD_PORT(0x80659A84, 0x806555FC, 0x806590F0, 0x80647D9C));

        RKNetController_ProcessRACEPacket(
            rkNetController, playerAid, packet, packetSize
        );
    }
)};

#endif

} // namespace wwfc::Security

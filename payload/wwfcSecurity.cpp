#include "mkwSecurity.hpp"
#include "wwfcLog.hpp"
#include "wwfcPatch.hpp"

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

// CLIENT TO CLIENT VULNERABILITY
// Patch for the Mario Kart Wii Race packet exploit. This was the first RCE
// exploit discovered in a Wii game. Originally discovered by XeR, but then
// rediscovered by Star, who reported the exploit and then released it.
// CVE-ID: CVE-2023-35856
// https://cve.mitre.org/cgi-bin/cvename.cgi?name=CVE-2023-35856
WWFC_DEFINE_PATCH = {Patch::BranchWithCTR( //
    WWFC_PATCH_LEVEL_CRITICAL, //
    RMCXD_PORT(0x80658604, 0x8065417C, 0x80657C70, 0x8064691C), //
    [](mkw::Net::NetController* netController, mkw::Net::RacePacket* racePacket,
       u32 packetSize, u32 _, u8 playerAid) -> void {
    if (packetSize < sizeof(mkw::Net::RacePacket)) {
        WWFC_LOG_WARN_FMT(
            "Invalid Race packet from aid %u (insufficient size)", playerAid
        );

        netController->reportAndKick("wl:bad_packet", playerAid);
        GPReport::ReportB64Encode("wl:bad_packet_data", racePacket, packetSize);

        return;
    }

    LONGCALL u32 NETCalcCRC32( //
        const void* data, u32 size
    ) AT(RMCXD_PORT(0x801D1CA0, 0x801D1C00, 0x801D1BC0, 0x801D1FFC));

    u32 savedChecksum = racePacket->checksum;
    racePacket->checksum = 0;
    u32 realChecksum = NETCalcCRC32(racePacket, packetSize);
    racePacket->checksum = savedChecksum;

    if (realChecksum != savedChecksum) {
        WWFC_LOG_WARN_FMT(
            "Invalid Race packet from aid %u (checksum mismatch)", playerAid
        );

        return;
    }

    if (!mkw::Security::IsRacePacketValid(racePacket, packetSize, playerAid)) {
        WWFC_LOG_WARN_FMT(
            "Invalid Race packet from aid %u (malicious packet)", playerAid
        );

        netController->reportAndKick("wl:bad_packet", playerAid);
        GPReport::ReportB64Encode("wl:bad_packet_data", racePacket, packetSize);

        return;
    }

    netController->processRacePacket(playerAid, racePacket, packetSize);
}
)};

#endif

} // namespace wwfc::Security

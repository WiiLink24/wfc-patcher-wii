#include "import/mkwNet.hpp"
#include "import/mkwRegistry.hpp"
#include "import/mkwSystem.hpp"
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

static bool
IsPacketSizeValid(mkw::Net::RACEPacket::EType packetType, u8 packetSize)
{
    using namespace mkw::Net;

    if (packetType >= RACEPacket::RACEHEADER_1 &&
        packetType <= RACEPacket::EVENT) {
        if (packetSize == 0) {
            return true;
        }
    }

    extern u32 packetBufferSizes[sizeof(RACEPacket::sizes)] AT(
        RMCXD_PORT(0x8089A194, 0x80895AC4, 0x808992F4, 0x808885CC)
    );

    switch (packetType) {
    case RACEPacket::HEADER: {
        if (packetSize < sizeof(RACEPacket) ||
            packetSize > packetBufferSizes[RACEPacket::HEADER]) {
            return false;
        }

        return true;
    }
    case RACEPacket::RACEHEADER_1: {
        if (packetSize < 0x28 ||
            packetSize > packetBufferSizes[RACEPacket::RACEHEADER_1]) {
            return false;
        }

        return true;
    }
    case RACEPacket::RACEHEADER_2: {
        if (packetSize < 0x28 ||
            packetSize > packetBufferSizes[RACEPacket::RACEHEADER_2]) {
            return false;
        }

        return true;
    }
    case RACEPacket::ROOM_SELECT: {
        // 'ROOM' packet
        if (packetSize < sizeof(SELECTHandler::Packet)) {
            return packetSize == 0x04;
        }

        // 'SELECT' packet
        if (packetSize > packetBufferSizes[RACEPacket::ROOM_SELECT]) {
            return false;
        }

        return true;
    }
    case RACEPacket::RACEDATA: {
        if (packetSize < 0x40 ||
            packetSize > packetBufferSizes[RACEPacket::RACEDATA] ||
            packetSize % 0x40) {
            return false;
        }

        return true;
    }
    case RACEPacket::USER: {
        if (packetSize < sizeof(USERPacket) ||
            packetSize > packetBufferSizes[RACEPacket::USER]) {
            return false;
        }

        return true;
    }
    case RACEPacket::ITEM: {
        if (packetSize < 0x08 ||
            packetSize > packetBufferSizes[RACEPacket::ITEM] ||
            packetSize % 0x08) {
            return false;
        }

        return true;
    }
    case RACEPacket::EVENT: {
        // SECURITY TODO: There is some kind of (harmless?) overflow in EVENT,
        // not patched by Wiimmfi or CTGP
        if (packetSize < 0x18 ||
            packetSize > packetBufferSizes[RACEPacket::EVENT]) {
            return false;
        }

        return true;
    }
    default: {
        return false;
    }
    }
}

static bool IsPacketDataValid(
    mkw::Net::RACEPacket::EType packetType, const void* packet, u8 packetSize
)
{
    using namespace mkw::Net;
    using namespace mkw::Registry;
    using namespace mkw::System;

    switch (packetType) {
    case RACEPacket::HEADER: {
        return true;
    }
    case RACEPacket::RACEHEADER_1: {
        return true;
    }
    case RACEPacket::RACEHEADER_2: {
        return true;
    }
    case RACEPacket::ROOM_SELECT: {
        // 'ROOM' packet
        if (packetSize == 0x04) {
        }
        // 'SELECT' packet
        else {
            RKNetController* rkNetController = RKNetController::Instance();
            if (rkNetController->inVanillaMatch()) {
                RaceConfig::Scenario* scenario;
                EGG::SceneManager* sceneManager =
                    RKSystem::Instance().sceneManager();
                if (static_cast<RKScene::SceneID>(
                        sceneManager->getCurrentSceneID()
                    ) == RKScene::SceneID::Race) {
                    scenario = &RaceConfig::Instance()->raceScenario();
                } else {
                    scenario = &RaceConfig::Instance()->menuScenario();
                }

                // Ensure that a valid course was voted for
                const SELECTHandler::Packet* selectPacket =
                    reinterpret_cast<const SELECTHandler::Packet*>(packet);
                for (size_t n = 0;
                     n < sizeof(SELECTHandler::Packet::player) /
                             sizeof(SELECTHandler::Packet::Player);
                     n++) {
                    SELECTHandler::Packet::CourseVote courseVote =
                        selectPacket->player[n].courseVote;
                    if (courseVote == SELECTHandler::Packet::CourseVote::None ||
                        courseVote ==
                            SELECTHandler::Packet::CourseVote::Random) {
                        continue;
                    }

                    Course course = static_cast<Course>(courseVote);
                    if (scenario->isOnlineVersusRace()) {
                        if (!IsRaceCourse(course)) {
                            return false;
                        }
                    } else /* if (scenario->isOnlineBattle()) */ {
                        if (!IsBattleCourse(course)) {
                            return false;
                        }
                    }
                }

                // Ensure that a valid course was selected
                if (selectPacket->selectedCourse !=
                    SELECTHandler::Packet::SelectedCourse::None) {
                    Course course =
                        static_cast<Course>(selectPacket->selectedCourse);
                    if (scenario->isOnlineVersusRace()) {
                        if (!IsRaceCourse(course)) {
                            return false;
                        }
                    } else /* if (scenario->isOnlineBattle()) */ {
                        if (!IsBattleCourse(course)) {
                            return false;
                        }
                    }
                }
            }
        }

        return true;
    }
    case RACEPacket::RACEDATA: {
        return true;
    }
    case RACEPacket::USER: {
        const USERPacket* userPacket =
            reinterpret_cast<const USERPacket*>(packet);

        // Mii count must be 2
        if (userPacket->miiGroupCount != 2) {
            return false;
        }

        return true;
    }
    case RACEPacket::ITEM: {
        return true;
    }
    case RACEPacket::EVENT: {
        return true;
    }
    default: {
        return false;
    }
    }
}

// CLIENT TO CLIENT VULNERABILITY
// Patch for Mario Kart Wii RACE exploit. This was the first RCE exploit
// discovered in a Wii game. Originally discovered by XeR, but then
// rediscovered by Star, who reported the exploit and then released it.
// CVE-ID: CVE-2023-35856
// https://cve.mitre.org/cgi-bin/cvename.cgi?name=CVE-2023-35856
static bool
IsRACEPacketValid(const mkw::Net::RACEPacket* racePacket, u32 racePacketSize)
{
    using namespace mkw::Net;

    if (racePacketSize < sizeof(RACEPacket)) {
        return false;
    }

    // Validate the size of each packet
    u32 expectedPacketSize = 0;
    for (size_t n = 0; n < sizeof(RACEPacket::sizes); n++) {
        RACEPacket::EType packetType = static_cast<RACEPacket::EType>(n);
        u8 packetSize = racePacket->sizes[n];

        if (!IsPacketSizeValid(packetType, packetSize)) {
            return false;
        }

        expectedPacketSize += packetSize;
    }

    // Ensure that the packet is large enough to hold the data
    if (racePacketSize < expectedPacketSize) {
        return false;
    }

    // Validate the data of each packet
    expectedPacketSize = 0;
    for (size_t n = 0; n < sizeof(RACEPacket::sizes); n++) {
        RACEPacket::EType packetType = static_cast<RACEPacket::EType>(n);
        const void* packet =
            reinterpret_cast<const char*>(racePacket) + expectedPacketSize;
        u8 packetSize = racePacket->sizes[n];

        if (packetSize != 0 &&
            !IsPacketDataValid(packetType, packet, packetSize)) {
            return false;
        }

        expectedPacketSize += packetSize;
    }

    return true;
}

WWFC_DEFINE_PATCH = {Patch::BranchWithCTR( //
    WWFC_PATCH_LEVEL_CRITICAL, //
    RMCXD_PORT(0x80658604, 0x8065417C, 0x80657C70, 0x8064691C), //
    [](mkw::Net::RKNetController* rkNetController,
       mkw::Net::RACEPacket* racePacket, u32 packetSize, u32 _,
       u8 playerAid) -> void {
        if (!IsRACEPacketValid(racePacket, packetSize)) {
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

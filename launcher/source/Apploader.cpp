#include "Apploader.hpp"
#include "DI.hpp"
#include "ES.hpp"
#include "GameAddresses.hpp"
#include "IOS.hpp"
#include "Scene.hpp"
#include "Stage1Payload.hpp"
#include "Util.hpp"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ogc/cache.h>
#include <ogc/ios.h>
#include <ogc/irq.h>
#include <ogc/lwp.h>
#include <ogc/system.h>
#include <ogc/video.h>
#include <unistd.h>
#include <wiiuse/wpad.h>

static constexpr u32 LOAD_DOL_ADDRESS = 0x80901000;
static constexpr u32 LOAD_DOL_MAXLEN = 0x00900000;

struct DOL {
    static constexpr u32 SECTION_COUNT = 7 + 11;

    u32 section[SECTION_COUNT];
    u32 sectionAddr[SECTION_COUNT];
    u32 sectionSize[SECTION_COUNT];

    u32 bssAddr;
    u32 bssSize;
    u32 entryPoint;
    u32 pad[0x1C / 4];

    u32 GetReal(u32 offset) const
    {
        for (u32 i = 0; i < DOL::SECTION_COUNT; i++) {
            u32 sectOff = section[i];
            u32 sectSize = sectionSize[i];
            if (offset >= sectOff && offset - sectOff < sectSize) {
                u32 sectAddr = sectionAddr[i];
                return sectAddr + (offset - sectOff);
            }
        }

        std::fprintf(stderr, "Invalid DOL section (1)\n");
        std::exit(EXIT_FAILURE);
    }

    u32 GetDol(u32 offset) const
    {
        for (u32 i = 0; i < DOL::SECTION_COUNT; i++) {
            u32 sectAddr = sectionAddr[i];
            u32 sectSize = sectionSize[i];
            if (offset >= sectAddr && offset - sectAddr < sectSize) {
                u32 sectOff = section[i];
                return sectOff + (offset - sectAddr);
            }
        }

        std::fprintf(stderr, "Invalid DOL section (2)\n");
        std::exit(EXIT_FAILURE);
    }

    u32 GetDolMem(u32 offset) const
    {
        return u32(this) + GetDol(offset);
    }
};

struct PartitionGroup {
    u32 count;
    u32 shiftedOffset;
};

static_assert(sizeof(PartitionGroup) == 0x8);

enum class PartitionType : u32 {
    DATA = 0,
};

struct PartitionInfo {
    u32 shiftedOffset;
    PartitionType type;
};

static_assert(sizeof(PartitionInfo) == 0x8);

struct PartitionOffsets {
    u32 dolOffset;
    u32 fstOffset;
    u32 fstSize;
    u32 fstMaxSize;
    u8 fill[0x10]; // Alignment
};

static_assert(sizeof(PartitionOffsets) == 0x20);

struct Stage1Param {
    u32 p_block;

    u32 p_NHTTPCreateRequest;
    u32 p_NHTTPSendRequestAsync;
    u32 p_NHTTPDestroyResponse;

    u32 p_allocator;
    u32 p_dwcError;

    char title[9];
    u8 finished;
    u8 padding[2];
    u32 p_stage1;
};

extern "C" {
void RunDOL(const DOL* dol);
void RunDOLEnd();

extern u32 g_wwfcPatchStart[];
extern u32 g_wwfcPatchLoadAuthWorkReq[];
extern u32 g_wwfcPatchAuthExit[];
extern Stage1Param g_wwfcPatchStage1Data;
extern u32 g_wwfcPatchEnd[];
}

static Apploader::State s_state = Apploader::State::INITIALIZING;
static bool s_shutdown = false;

Apploader::State Apploader::GetState()
{
    return s_state;
}

static void PatchAndLaunchDol(
    DOL* dol, const GameAddresses* game, const void* bi2, u32 mem1Hi
)
{
    // Add stage 1
    const u32 stage1Start = AlignDown(mem1Hi - sizeof(Stage1Payload), 32);
    std::memcpy(
        reinterpret_cast<void*>(stage1Start), Stage1Payload,
        sizeof(Stage1Payload)
    );
    DCFlushRange(reinterpret_cast<void*>(stage1Start), sizeof(Stage1Payload));
    ICInvalidateRange(
        reinterpret_cast<void*>(stage1Start), sizeof(Stage1Payload)
    );

    const u32 payloadBlock = stage1Start - 0x20000;
    const u32 wwfcAsm = payloadBlock - 0x100;

    const u32 start = u32(g_wwfcPatchStart);
    const u32 loadAuthWork = u32(g_wwfcPatchLoadAuthWorkReq);
    const u32 authExit = u32(g_wwfcPatchAuthExit);
    const u32 end = u32(g_wwfcPatchEnd);

    // Patch WWFC
    const u32 stage1DataAddr = wwfcAsm + (u32(&g_wwfcPatchStage1Data) - start);
    MaskU32(start + 0, 0xFFFF, (stage1DataAddr >> 16) & 0xFFFF);
    MaskU32(start + 4, 0xFFFF, stage1DataAddr & 0xFFFF);

    WriteU32(loadAuthWork + 0, game->loadAuthRequestAsm[0]);
    WriteU32(loadAuthWork + 4, game->loadAuthRequestAsm[1]);
    WriteU32(loadAuthWork + 8, game->loadAuthRequestAsm[2]);

    WriteU32(authExit + 0, ReadU32(dol->GetDolMem(game->addrAuthSendRequest)));
    WriteU32(
        authExit + 4,
        MakeBranch(
            wwfcAsm + (authExit - start) + 4, game->addrAuthSendRequest + 4
        )
    );

    g_wwfcPatchStage1Data = {
        .p_block = payloadBlock,
        .p_NHTTPCreateRequest = game->addrNHTTPCreateRequest,
        .p_NHTTPSendRequestAsync = game->addrNHTTPSendRequest,
        .p_NHTTPDestroyResponse = game->addrNHTTPDestroyResponse,
        .p_allocator = 0,
        .p_dwcError = game->addrNASError,
        .title = {},
        .finished = 0,
        .padding = {},
        .p_stage1 = stage1Start,
    };
    std::strncpy(g_wwfcPatchStage1Data.title, game->gameId, 9);

    std::memcpy(
        reinterpret_cast<void*>(wwfcAsm), g_wwfcPatchStart, end - start
    );
    DCFlushRange(reinterpret_cast<void*>(wwfcAsm), end - start);
    ICInvalidateRange(reinterpret_cast<void*>(wwfcAsm), end - start);

    // Add hook to game
    WriteU32(
        dol->GetDolMem(game->addrAuthSendRequest),
        MakeBranch(game->addrAuthSendRequest, wwfcAsm)
    );

    // Skip DNS cache in login
    WriteU32(
        dol->GetDolMem(game->addrSkipDNSCache),
        MakeBranch(game->addrSkipDNSCache, game->addrSkipDNSCacheContinue)
    );

    // Patch available domain
    std::strcpy(
        reinterpret_cast<char*>(dol->GetDolMem(game->addrAvailableDomain)),
        "%s.av.gs.wiilink24.com"
    );

    // New MEM1 arena high, excluding BI2
    mem1Hi = wwfcAsm;

    // Copy BI2
    u32 bi2Dest = wwfcAsm - 0x2000;
    std::memcpy(reinterpret_cast<void*>(bi2Dest), bi2, 0x2000);
    DCStoreRange(reinterpret_cast<void*>(bi2Dest), 0x2000);

    std::memcpy(
        reinterpret_cast<void*>(0x80900000), (void*) RunDOL,
        u32(RunDOLEnd) - u32(RunDOL)
    );
    DCStoreRange(
        reinterpret_cast<void*>(0x80900000), u32(RunDOLEnd) - u32(RunDOL)
    );
    ICInvalidateRange(
        reinterpret_cast<void*>(0x80900000), u32(RunDOLEnd) - u32(RunDOL)
    );

    __IOS_ShutdownSubsystems();
    for (u32 i = 0; i < 32; i++) {
        IOS_CloseAsync(i, nullptr, nullptr);
    }

    Scene::Shutdown(Scene::ShutdownType::LAUNCH);

    IRQ_Disable();

    SYS_ResetSystem(SYS_SHUTDOWN, 0, 0);

    // Stub exception handlers
    static constexpr u32 EXCEPTION_HANDLERS[] = {
        0x0100, 0x0200, 0x0300, 0x0400, 0x0500, 0x0600, 0x0700,
        0x0800, 0x0900, 0x0D00, 0x0F00, 0x1300, 0x1400, 0x1700,
    };

    for (u32 i = 0; i < sizeof(EXCEPTION_HANDLERS) / sizeof(u32); i++) {
        WriteU32(0x80000000 + EXCEPTION_HANDLERS[i], 0x48000000);
    }

    // Reset the DSP; LibOGC apps like the HBC cannot initialize it properly,
    // but the SDK can
    WriteU32(0xCD006C00, 0x00000000);

    WriteU32(0x80000034, 0); // Arena High

    WriteU32(0x800000EC, 0x81800000); // Dev Debugger Monitor Address
    WriteU32(0x800000F0, 0x01800000); // Simulated Memory Size
    WriteU32(0x800000F4, bi2Dest); // Pointer to data from partition's bi2.bin
    WriteU32(0x800000F8, 0x0E7BE2C0); // Console Bus Speed
    WriteU32(0x800000FC, 0x2B73A840); // Console CPU Speed

    WriteU32(0x80003110, wwfcAsm); // MEM1 Arena End
    WriteU32(0x80003124, 0x90000800); // Usable MEM2 Start
    WriteU32(0x80003180, ReadU32(0x80000000)); // Game ID
    WriteU32(0x80003188, ReadU32(0x80003140)); // IOS Version + Revision

    // Check dual layer disc
    if (ReadU32(bi2Dest + 0x30) == 0x7ED40000) {
        WriteU8(0x8000319C, 0x81); // Dual layer
    } else {
        WriteU8(0x8000319C, 0x80); // Single layer
    }

    DCStoreRange(reinterpret_cast<void*>(0x80000000), 0x3400);
    ICInvalidateRange(reinterpret_cast<void*>(0x80000000), 0x3400);

    ((void (*)(const DOL* dol)) 0x80900000)(dol);

    while (true) {
    }
}

static bool NeedIOSReload(const GameAddresses* game, u32 iosExpected)
{
    u32 iosCurrent = IOS_GetVersion();

    if (iosCurrent == iosExpected) {
        return false;
    }

    // Mario Kart Wii can run under IOS58
    if (std::memcmp(game->gameId, "RMC", 3) == 0 && iosCurrent == 58) {
        return false;
    }

    return true;
}

static Apploader::State LaunchDisc()
{
#define CHECK_SHUTDOWN()                                                       \
    if (s_shutdown) {                                                          \
        return Apploader::State::SHUTTING_DOWN;                                \
    }

    std::printf("Reading the disc, please wait...\n");

    DI::DiskID diskId = {};
    auto diRet = DI::ReadDiskID(&diskId);
    if (diRet == DI::DIError::DRIVE) {
        // The drive probably hasn't been spun up yet
        s_state = Apploader::State::DISC_SPINUP;

        diRet = DI::Reset(true);
        CHECK_SHUTDOWN();
        if (diRet == DI::DIError::OK) {
            diRet = DI::ReadDiskID(&diskId);
        }
    }

    if (diRet != DI::DIError::OK) {
        std::fprintf(stderr, "Failed to init DI: 0x%X\n", u32(diRet));
        return Apploader::State::READ_ERROR;
    }
    CHECK_SHUTDOWN();

    char gameId[16];
    std::snprintf(
        gameId, sizeof(gameId), "%c%c%c%cD%02x", diskId.gameId[0],
        diskId.gameId[1], diskId.gameId[2], diskId.gameId[3], diskId.discVer
    );

    std::printf("Game ID/Rev: %s\n", gameId);

    const GameAddresses* game = nullptr;
    for (u32 i = 0; i < GameAddressesListSize; i++) {
        if (std::strcmp(gameId, GameAddressesList[i].gameId) == 0) {
            game = &GameAddressesList[i];
            break;
        }
    }

    if (game == nullptr) {
        std::fprintf(stderr, "Game not supported\n");
        return Apploader::State::UNSUPPORTED_GAME;
    }

    s_state = Apploader::State::READING_DISC;

    // Find the game partition
    PartitionGroup groups[4] alignas(0x20);
    diRet = DI::UnencryptedRead(groups, sizeof(groups), 0x40000 >> 2);
    if (diRet != DI::DIError::OK) {
        std::fprintf(
            stderr, "Failed to read partition groups: 0x%X\n", u32(diRet)
        );
        return Apploader::State::READ_ERROR;
    }
    CHECK_SHUTDOWN();

    u32 partOffset = 0;

    for (u32 i = 0; i < 4 && partOffset == 0; i++) {
        u32 partitionCount = groups[i].count;
        u32 offset = groups[i].shiftedOffset;

        if (partitionCount == 0 || partitionCount > 4 || offset == 0) {
            continue;
        }

        PartitionInfo partitions[4] alignas(0x20);
        diRet = DI::UnencryptedRead(partitions, sizeof(partitions), offset);
        if (diRet != DI::DIError::OK) {
            std::fprintf(
                stderr, "Failed to read partition info at offset %08X: 0x%X\n",
                offset, u32(diRet)
            );
            return Apploader::State::READ_ERROR;
        }
        CHECK_SHUTDOWN();

        for (u32 j = 0; j < partitionCount; j++) {
            if (partitions[j].type == PartitionType::DATA) {
                partOffset = partitions[j].shiftedOffset;
                break;
            }
        }
    }

    if (partOffset == 0) {
        std::fprintf(stderr, "Failed to find game partition\n");
        return Apploader::State::READ_ERROR;
    }

    // If the game is Korean or Chinese, we may need IOS patches
    if ((diskId.gameId[3] == 'K' || diskId.gameId[3] == 'C') &&
        !IOS::IsDolphin()) {
        IOS::PatchIOS(IOS::PATCH_IOSC_KOREAN_KEY);
    }

    ES::TMDFixed<512> tmd alignas(32) = {};

    ES::ESError esRet = ES::ESError::OK;
    diRet = DI::OpenPartition(partOffset, &tmd, &esRet);
    if (diRet != DI::DIError::OK) {
        std::fprintf(
            stderr, "Failed to open partition at offset %08X: 0x%X, ES: %d\n",
            partOffset, u32(diRet), s32(esRet)
        );
        return Apploader::State::READ_ERROR;
    }

    std::printf("Opened partition at offset %08X\n", partOffset);
    CHECK_SHUTDOWN();

    PartitionOffsets hdrOffsets alignas(32) = {};
    diRet = DI::Read(&hdrOffsets, sizeof(hdrOffsets), 0x420 >> 2);
    if (diRet != DI::DIError::OK) {
        std::fprintf(
            stderr, "Failed to read from %08X: 0x%X\n", partOffset, u32(diRet)
        );
        return Apploader::State::READ_ERROR;
    }
    CHECK_SHUTDOWN();

    // Read the DOL
    DOL* dol = reinterpret_cast<DOL*>(LOAD_DOL_ADDRESS);

    diRet = DI::Read(dol, sizeof(DOL), hdrOffsets.dolOffset);
    if (diRet != DI::DIError::OK) {
        std::fprintf(stderr, "Failed to read DOL header: 0x%X\n", u32(diRet));
        return Apploader::State::READ_ERROR;
    }

    for (u32 i = 0; i < DOL::SECTION_COUNT; i++) {
        if (dol->sectionSize[i] == 0) {
            continue;
        }

        CHECK_SHUTDOWN();

        std::printf(
            "DOL section (%02u): %08X, %08X, %08X\n", i, dol->section[i],
            dol->sectionAddr[i], dol->sectionSize[i]
        );

        dol->sectionSize[i] = AlignUp(dol->sectionSize[i], 32);

        if (!IsAligned(dol->section[i], 32) ||
            !IsAligned(dol->sectionAddr[i], 32) ||
            !IsAligned(dol->sectionSize[i], 4)) {
            std::fprintf(stderr, "DOL section (%02u) has bad alignment\n", i);
            return Apploader::State::READ_ERROR;
        }

        if (!CheckBounds(
                sizeof(DOL), LOAD_DOL_MAXLEN - sizeof(DOL), dol->section[i],
                dol->sectionSize[i]
            ) ||
            !CheckBounds(
                0x80001800, 0x80900000 - 0x80001800, dol->sectionAddr[i],
                dol->sectionSize[i]
            )) {
            std::fprintf(stderr, "DOL section (%02u) out of bounds\n", i);
            return Apploader::State::READ_ERROR;
        }

        diRet = DI::Read(
            reinterpret_cast<void*>(LOAD_DOL_ADDRESS + dol->section[i]),
            dol->sectionSize[i], hdrOffsets.dolOffset + (dol->section[i] >> 2)
        );
        if (diRet != DI::DIError::OK) {
            std::fprintf(
                stderr, "Failed to read DOL section (%u): 0x%X\n", i, u32(diRet)
            );
            return Apploader::State::READ_ERROR;
        }
    }

    // Read the FST
    const u32 fstSize = hdrOffsets.fstSize << 2;
    const u32 fstDest = AlignDown(0x81800000 - fstSize, 32);
    if (fstDest < 0x81700000) {
        std::fprintf(stderr, "FST size is too large\n");
        return Apploader::State::READ_ERROR;
    }

    diRet = DI::Read(
        reinterpret_cast<void*>(fstDest), AlignUp(fstSize, 32),
        hdrOffsets.fstOffset
    );
    if (diRet != DI::DIError::OK) {
        std::fprintf(stderr, "Failed to read FST: 0x%X\n", u32(diRet));
        return Apploader::State::READ_ERROR;
    }
    CHECK_SHUTDOWN();

    u8 bi2Data[0x2000] alignas(32);
    diRet = DI::Read(bi2Data, 0x2000, 0x440);
    if (diRet != DI::DIError::OK) {
        std::fprintf(stderr, "Failed to read BI2: 0x%X\n", u32(diRet));
        return Apploader::State::READ_ERROR;
    }
    CHECK_SHUTDOWN();

    s_state = Apploader::State::LAUNCHING;

    WPAD_Shutdown();

    if (NeedIOSReload(game, U64Lo(tmd.iosTitleId))) {
        IOS::WaitForPatchIOS();

        std::printf("Reloading into IOS%d\n", U64Lo(tmd.iosTitleId));

        LWP_SetThreadPriority(LWP_GetSelf(), 1);
        s32 ret = IOS_ReloadIOS(U64Lo(tmd.iosTitleId));
        if (ret != 0) {
            std::fprintf(stderr, "Failed to reload IOS: %d\n", ret);
            return Apploader::State::FATAL_ERROR;
        }
        LWP_SetThreadPriority(LWP_GetSelf(), 80);

        // Reopen the partition
        if (!DI::Init()) {
            std::printf("Failed to reopen DI\n");
            return Apploader::State::FATAL_ERROR;
        }

        if ((diskId.gameId[3] == 'K' || diskId.gameId[3] == 'C') &&
            !IOS::IsDolphin()) {
            // Needs Korean key
            IOS::PatchIOS(IOS::PATCH_IOSC_KOREAN_KEY);
        }

        // Read the disk ID to the top of MEM1
        diRet = DI::ReadDiskID(reinterpret_cast<DI::DiskID*>(0x80000000));
        if (diRet != DI::DIError::OK) {
            std::fprintf(
                stderr, "Failed to reread disk ID: 0x%X\n", u32(diRet)
            );
            return Apploader::State::FATAL_ERROR;
        }

        if (!IOS::IsDolphin()) {
            diRet =
                DI::OpenPartitionWithTmdAndTicketView(partOffset, &tmd, &esRet);
        } else {
            // Dolphin doesn't implement that version of OpenPartition
            diRet = DI::OpenPartition(partOffset, &tmd, &esRet);
        }

        if (diRet != DI::DIError::OK) {
            std::fprintf(
                stderr,
                "Failed to reopen partition at offset %08X: 0x%X, ES: %d\n",
                partOffset, u32(diRet), s32(esRet)
            );
            return Apploader::State::FATAL_ERROR;
        }

        std::printf("Reopened partition at offset %08X\n", partOffset);
    } else {
        std::memcpy(
            reinterpret_cast<void*>(0x80000000), &diskId, sizeof(DI::DiskID)
        );
    }

    WriteU32(0x80000038, fstDest); // Start of FST (varies in all games)
    PatchAndLaunchDol(dol, game, bi2Data, fstDest);

    // Should never be reached
    return Apploader::State::FATAL_ERROR;
#undef CHECK_SHUTDOWN
}

static void* ApploaderProcess(void* arg)
{
#define CHECK_SHUTDOWN()                                                       \
    if (s_shutdown) {                                                          \
        s_state = Apploader::State::SHUTTING_DOWN;                             \
        return nullptr;                                                        \
    }

    while (!s_shutdown) {
        // Wait for a disc to be inserted
        bool printed = false;
        for (;;) {
            u32 cover;
            DI::DIError diRet = DI::GetCoverRegister(&cover);
            if (diRet != DI::DIError::OK) {
                std::fprintf(
                    stderr, "Failed to get cover register: 0x%X\n", u32(diRet)
                );
                s_state = Apploader::State::FATAL_ERROR;
                return nullptr;
            }
            CHECK_SHUTDOWN();

            // Check if the cover is closed
            if ((cover & DI::DICVR_CVR) == 0) {
                break;
            }

            if (!printed) {
                std::printf("Waiting for a disc to be inserted.\n");
                printed = true;
            }

            s_state = Apploader::State::NO_DISC;

            // Sleep for 8 ms
            usleep(8000);
            CHECK_SHUTDOWN();
        }

        s_state = LaunchDisc();
        if (s_state == Apploader::State::FATAL_ERROR) {
            return nullptr;
        }

        if (s_shutdown) {
            DI::Reset(false);
            return nullptr;
        }

        // Stop the motor for an error
        DI::StopMotor(false, false);

        std::printf("Waiting for the disc to be ejected.\n");

        // Wait for the disc to be removed
        for (;;) {
            u32 cover = 0;
            DI::DIError diRet = DI::GetCoverRegister(&cover);
            if (diRet != DI::DIError::OK) {
                std::fprintf(
                    stderr, "Failed to get cover register: 0x%X\n", u32(diRet)
                );
                s_state = Apploader::State::FATAL_ERROR;
                return nullptr;
            }
            CHECK_SHUTDOWN();

            // Check if the cover is open
            if ((cover & DI::DICVR_CVR) == 1) {
                break;
            }

            // Sleep for 8 ms
            usleep(8000);
            CHECK_SHUTDOWN();
        }
    }

    return nullptr;
#undef CHECK_SHUTDOWN
}

static lwp_t s_thread;
static bool s_threadStarted = false;

void Apploader::StartThread()
{
    if (s_threadStarted) {
        return;
    }

    s_threadStarted = true;
    LWP_CreateThread(
        &s_thread, ApploaderProcess, nullptr, nullptr, 0x40000, 20
    );
}

void Apploader::ShutdownAsync()
{
    s_shutdown = true;
}

void Apploader::Shutdown()
{
    s_shutdown = true;
    if (s_threadStarted) {
        LWP_JoinThread(s_thread, nullptr);
    }
}
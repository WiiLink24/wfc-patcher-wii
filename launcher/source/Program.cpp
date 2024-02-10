#include "DI.hpp"
#include "GameAddresses.hpp"
#include "Stage1Payload.hpp"
#include "Util.hpp"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ogc/cache.h>
#include <ogc/consol.h>
#include <ogc/es.h>
#include <ogc/gx.h>
#include <ogc/ios.h>
#include <ogc/ipc.h>
#include <ogc/irq.h>
#include <ogc/system.h>
#include <ogc/video.h>

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

extern "C" {
void RunDOL(const DOL* dol);
void RunDOLEnd();

extern u32 WWFCPatchStart[];
extern u32 WWFCPatch_LoadAuthWorkReq[];
extern u32 WWFCPatch_AuthExit[];
extern u32 WWFCPatch_Stage1Data[];
extern u32 WWFCPatchEnd[];
}

static void* s_xfb = nullptr;
static GXRModeObj* s_rmode = nullptr;

static u32 DolToReal(const DOL* dol, u32 offset)
{
    for (u32 i = 0; i < DOL::SECTION_COUNT; i++) {
        u32 sectOff = dol->section[i];
        u32 sectSize = dol->sectionSize[i];
        if (offset >= sectOff && offset - sectOff < sectSize) {
            u32 sectAddr = dol->sectionAddr[i];
            return sectAddr + (offset - sectOff);
        }
    }

    std::fprintf(stderr, "Invalid DOL section (1)\n");
    std::exit(EXIT_FAILURE);
}

static u32 RealToDol(const DOL* dol, u32 offset)
{
    for (u32 i = 0; i < DOL::SECTION_COUNT; i++) {
        u32 sectAddr = dol->sectionAddr[i];
        u32 sectSize = dol->sectionSize[i];
        if (offset >= sectAddr && offset - sectAddr < sectSize) {
            u32 sectOff = dol->section[i];
            return sectOff + (offset - sectAddr);
        }
    }

    std::fprintf(stderr, "Invalid DOL section (2)\n");
    std::exit(EXIT_FAILURE);
}

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

bool IsDolphin()
{
    static bool isDolphin = IsDolphinImpl();
    return isDolphin;
}

int main(int argc, char** argv)
{
    (void) argc;
    (void) argv;

    // Initialize video
    VIDEO_Init();
    s_rmode = VIDEO_GetPreferredMode(nullptr);
    s_xfb = MEM_K0_TO_K1(SYS_AllocateFramebuffer(s_rmode));
    VIDEO_Configure(s_rmode);

    // Initialize LibOGC console
    console_init(
        s_xfb, 20, 20, s_rmode->fbWidth, s_rmode->xfbHeight,
        s_rmode->fbWidth * VI_DISPLAY_PIX_SZ
    );
    VIDEO_SetNextFramebuffer(s_xfb);
    VIDEO_SetBlack(false);
    VIDEO_Flush();

    // Adjust to row 2 column 0
    std::printf("\x1b[%d;%dH", 2, 0);

    // Print logo
    std::printf("WiiLink WFC Launcher\n\n");

    // Initialize DI
    if (!DI::Init()) {
        std::printf("Failed to initialize DI\n");
        return EXIT_FAILURE;
    }

    DI::DiskID* diskId = reinterpret_cast<DI::DiskID*>(0x80000000);

    std::printf("Reading the disc, please wait...\n");
    auto retDi = DI::ReadDiskID(diskId);
    if (retDi == DI::DIError::DRIVE) {
        // The drive probably hasn't been spun up yet
        retDi = DI::Reset(true);
        if (retDi == DI::DIError::OK) {
            retDi = DI::ReadDiskID(diskId);
        }
    }

    if (retDi != DI::DIError::OK) {
        std::fprintf(stderr, "Failed to init DI: 0x%X\n", u32(retDi));
        return EXIT_FAILURE;
    }

    char gameId[16];
    std::snprintf(
        gameId, sizeof(gameId), "%c%c%c%cD%02x", diskId->gameId[0],
        diskId->gameId[1], diskId->gameId[2], diskId->gameId[3], diskId->discVer
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
        return EXIT_FAILURE;
    }

    // Find the game partition
    PartitionGroup groups[4] alignas(0x20);
    retDi = DI::UnencryptedRead(groups, sizeof(groups), 0x40000 >> 2);
    if (retDi != DI::DIError::OK) {
        std::fprintf(
            stderr, "Failed to read partition groups: 0x%X\n", u32(retDi)
        );
        return EXIT_FAILURE;
    }

    u32 partOffset = 0;

    for (u32 i = 0; i < 4 && partOffset == 0; i++) {
        u32 partitionCount = groups[i].count;
        u32 offset = groups[i].shiftedOffset;

        if (partitionCount == 0 || partitionCount > 4 || offset == 0) {
            continue;
        }

        PartitionInfo partitions[4] alignas(0x20);
        retDi = DI::UnencryptedRead(partitions, sizeof(partitions), offset);
        if (retDi != DI::DIError::OK) {
            std::fprintf(
                stderr, "Failed to read partition info at offset %08X: 0x%X\n",
                offset, u32(retDi)
            );
            return EXIT_FAILURE;
        }

        for (u32 j = 0; j < partitionCount; j++) {
            if (partitions[j].type == PartitionType::DATA) {
                partOffset = partitions[j].shiftedOffset;
                break;
            }
        }
    }

    if (partOffset == 0) {
        std::fprintf(stderr, "Failed to find game partition\n");
        return EXIT_FAILURE;
    }

    ES::TMDFixed<512> tmd alignas(32) = {};

    ES::ESError retEs;
    retDi = DI::OpenPartition(partOffset, &tmd, &retEs);
    if (retDi != DI::DIError::OK) {
        std::fprintf(
            stderr, "Failed to open partition at offset %08X: 0x%X, ES: %d\n",
            partOffset, u32(retDi), s32(retEs)
        );
        return EXIT_FAILURE;
    }

    std::printf("Opened partition at offset %08X\n", partOffset);

    PartitionOffsets hdrOffsets alignas(32) = {};
    retDi = DI::Read(&hdrOffsets, sizeof(hdrOffsets), 0x420 >> 2);
    if (retDi != DI::DIError::OK) {
        std::fprintf(
            stderr, "Failed to read from %08X: 0x%X\n", partOffset, u32(retDi)
        );
        return EXIT_FAILURE;
    }

    // Read the DOL
    DOL* dol = reinterpret_cast<DOL*>(LOAD_DOL_ADDRESS);

    retDi = DI::Read(dol, sizeof(DOL), hdrOffsets.dolOffset);
    if (retDi != DI::DIError::OK) {
        std::fprintf(stderr, "Failed to read DOL header: 0x%X\n", u32(retDi));
        return EXIT_FAILURE;
    }

    for (u32 i = 0; i < DOL::SECTION_COUNT; i++) {
        if (dol->sectionSize[i] == 0) {
            continue;
        }

        std::printf(
            "DOL section (%02u): %08X, %08X, %08X\n", i, dol->section[i],
            dol->sectionAddr[i], dol->sectionSize[i]
        );

        dol->sectionSize[i] = AlignUp(dol->sectionSize[i], 32);

        if (!IsAligned(dol->section[i], 32) ||
            !IsAligned(dol->sectionAddr[i], 32) ||
            !IsAligned(dol->sectionSize[i], 4)) {
            std::fprintf(stderr, "DOL section (%02u) has bad alignment\n", i);
            return EXIT_FAILURE;
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
            return EXIT_FAILURE;
        }

        retDi = DI::Read(
            reinterpret_cast<void*>(LOAD_DOL_ADDRESS + dol->section[i]),
            dol->sectionSize[i], hdrOffsets.dolOffset + (dol->section[i] >> 2)
        );
        if (retDi != DI::DIError::OK) {
            std::fprintf(
                stderr, "Failed to read DOL section (%u): 0x%X\n", i, u32(retDi)
            );
            return EXIT_FAILURE;
        }
    }

    // Read the FST
    u32 fstSize = hdrOffsets.fstSize << 2;
    u32 fstDest = AlignDown(0x81800000 - fstSize, 32);
    if (fstDest < 0x81700000) {
        std::fprintf(stderr, "FST size is too large\n");
        return EXIT_FAILURE;
    }

    retDi = DI::Read(
        reinterpret_cast<void*>(fstDest), AlignUp(fstSize, 32),
        hdrOffsets.fstOffset
    );
    if (retDi != DI::DIError::OK) {
        std::fprintf(stderr, "Failed to read FST: 0x%X\n", u32(retDi));
        return EXIT_FAILURE;
    }

    // Add stage 1
    u32 stage1Start = AlignDown(fstDest - sizeof(Stage1Payload), 32);
    std::memcpy(
        reinterpret_cast<void*>(stage1Start), Stage1Payload,
        sizeof(Stage1Payload)
    );
    DCFlushRange(reinterpret_cast<void*>(stage1Start), sizeof(Stage1Payload));
    ICInvalidateRange(
        reinterpret_cast<void*>(stage1Start), sizeof(Stage1Payload)
    );

    u32 payloadBlock = stage1Start - 0x20000;
    u32 wwfcAsm = payloadBlock - 0x100;

    // Patch WWFC
    u32 stage1DataOffset =
        wwfcAsm + (u32(WWFCPatch_Stage1Data) - u32(WWFCPatchStart));
    WWFCPatchStart[0] &= ~0xFFFF;
    WWFCPatchStart[0] |= u16(stage1DataOffset >> 16);
    WWFCPatchStart[1] &= ~0xFFFF;
    WWFCPatchStart[1] |= u16(stage1DataOffset);

    WWFCPatch_LoadAuthWorkReq[0] = game->loadAuthRequestAsm[0];
    WWFCPatch_LoadAuthWorkReq[1] = game->loadAuthRequestAsm[1];
    WWFCPatch_LoadAuthWorkReq[2] = game->loadAuthRequestAsm[2];
    if (WWFCPatch_LoadAuthWorkReq[2] == 0) {
        WWFCPatch_LoadAuthWorkReq[2] = 0x60000000;
    }

    u32 authExitOffset =
        wwfcAsm + (u32(WWFCPatch_AuthExit) - u32(WWFCPatchStart));
    WWFCPatch_AuthExit[0] =
        *(u32*) (u32(dol) + RealToDol(dol, game->addrAuthSendRequest));
    WWFCPatch_AuthExit[1] =
        0x48000000 |
        (((game->addrAuthSendRequest + 4) - (authExitOffset + 4)) & 0x3FFFFFF);

    WWFCPatch_Stage1Data[0] = payloadBlock;
    WWFCPatch_Stage1Data[1] = game->addrNHTTPCreateRequest;
    WWFCPatch_Stage1Data[2] = game->addrNHTTPSendRequest;
    WWFCPatch_Stage1Data[3] = game->addrNHTTPDestroyResponse;
    WWFCPatch_Stage1Data[5] = game->addrNASError;
    std::memset(WWFCPatch_Stage1Data + 6, 0, 9);
    std::strncpy(reinterpret_cast<char*>(WWFCPatch_Stage1Data + 6), gameId, 9);
    WWFCPatch_Stage1Data[9] = stage1Start;

    std::memcpy(
        reinterpret_cast<void*>(wwfcAsm), WWFCPatchStart,
        u32(WWFCPatchEnd) - u32(WWFCPatchStart)
    );
    DCFlushRange(
        reinterpret_cast<void*>(wwfcAsm),
        u32(WWFCPatchEnd) - u32(WWFCPatchStart)
    );
    ICInvalidateRange(
        reinterpret_cast<void*>(wwfcAsm),
        u32(WWFCPatchEnd) - u32(WWFCPatchStart)
    );

    // Read BI2
    u32 bi2 = wwfcAsm - 0x2000;
    retDi = DI::Read(reinterpret_cast<void*>(bi2), 0x2000, 0x440);
    if (retDi != DI::DIError::OK) {
        std::fprintf(stderr, "Failed to read BI2: 0x%X\n", u32(retDi));
        return EXIT_FAILURE;
    }

    std::printf("Reloading into IOS%d\n", U64Lo(tmd.iosTitleId));
    s32 ret = IOS_ReloadIOS(U64Lo(tmd.iosTitleId));
    if (ret != 0) {
        std::fprintf(stderr, "Failed to reload IOS: %d\n", ret);
        return EXIT_FAILURE;
    }

    // Reopen the partition
    if (!DI::Init()) {
        std::printf("Failed to reopen DI\n");
        return EXIT_FAILURE;
    }

    retDi = DI::ReadDiskID(diskId);
    if (retDi != DI::DIError::OK) {
        std::fprintf(stderr, "Failed to reread disk ID: 0x%X\n", u32(retDi));
        return EXIT_FAILURE;
    }

    if (!IsDolphin()) {
        retDi = DI::OpenPartitionWithTmdAndTicketView(partOffset, &tmd, &retEs);
    } else {
        // Dolphin doesn't implement that version of OpenPartition
        retDi = DI::OpenPartition(partOffset, &tmd, &retEs);
    }

    if (retDi != DI::DIError::OK) {
        std::fprintf(
            stderr, "Failed to reopen partition at offset %08X: 0x%X, ES: %d\n",
            partOffset, u32(retDi), s32(retEs)
        );
        return EXIT_FAILURE;
    }

    std::printf("Reopened partition at offset %08X\n", partOffset);

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

    VIDEO_SetBlack(true);
    VIDEO_Flush();
    VIDEO_WaitVSync();

    IRQ_Disable();

    SYS_ResetSystem(SYS_SHUTDOWN, 0, 0);

    // Add hook to game
    *(u32*) (u32(dol) + RealToDol(dol, game->addrAuthSendRequest)) =
        0x48000000 | ((wwfcAsm - game->addrAuthSendRequest) & 0x3FFFFFF);

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
    WriteU32(0x80000038, fstDest); // Start of FST (varies in all games)

    WriteU32(0x800000EC, 0x81800000); // Dev Debugger Monitor Address
    WriteU32(0x800000F0, 0x01800000); // Simulated Memory Size
    WriteU32(0x800000F4, bi2); // Pointer to data read from partition's bi2.bin
    WriteU32(0x800000F8, 0x0E7BE2C0); // Console Bus Speed
    WriteU32(0x800000FC, 0x2B73A840); // Console CPU Speed

    WriteU32(0x80003110, wwfcAsm); // MEM1 Arena End
    WriteU32(0x80003124, 0x90000800); // Usable MEM2 Start
    WriteU32(0x80003180, ReadU32(0x80000000)); // Game ID
    WriteU32(0x80003188, ReadU32(0x80003140)); // IOS Version + Revision

    // Check dual layer disc
    if (ReadU32(bi2 + 0x30) == 0x7ED40000) {
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
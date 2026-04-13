#if RMC || RMCN

#  if RMC
#    include "import/mkw/net/userHandler.hpp"
#  endif
#  include "import/mkw/nwc24.hpp"
#  include "import/rfl.h"
#  include "wwfcGPReport.hpp"
#  include "wwfcLibC.hpp"
#  include "wwfcPatch.hpp"

namespace wwfc::Mii
{

static void ClearMiiInfo(RFL::RFLiStoreData* miiData);

#  if RMC
static void ClearUserMiiInfo(mkw::NetUserHandler::Packet* packet)
{
    // Clear hidden Mii info
    // Mii Group count _should_ be 2, but could be higher if the code has been
    // messed with
    for (u32 i = 0; i < packet->miiGroupCount; i++) {
        // Check if this is a guest "no name" Mii
        u16 discard;
        if (!RFL::RFLSearchOfficialData(
                &packet->miiData[i].data.createID, &discard
            )) {
            // It's not a guest Mii
            ClearMiiInfo(&packet->miiData[i]);
        }
    }

    // Send User packet to the server
    wwfc::GPReport::ReportB64Encode(
        "wl:mkw_user", packet, sizeof(mkw::NetUserHandler::Packet)
    );
}

WWFC_DEFINE_PATCH = Patch::CallWithCTR(
    WWFC_PATCH_LEVEL_BUGFIX, //
    // RMC:NetUserHandler::createUserRecord
    RMCXD_PORT(0x80663178, 0x80661094, 0x806627E4, 0x80651490, 0x806636BC), //

    ASM_LAMBDA(
        ( : ASM_IMPORT(i, ClearUserMiiInfo)),
        // clang-format off
        mflr    r30;
        addi    r3, r31, 0x8;
        bl      %[ClearUserMiiInfo];
        mtlr    r30;
        // Function epilogue
        lwz     r31, 0x1C(r1);
        lwz     r30, 0x18(r1);
        lwz     r29, 0x14(r1);
        lwz     r0, 0x24(r1);
        blr;
        // clang-format on
    )
);
#  endif // RMC

static int RFLGetStoreDataOverride(RFL::RFLiStoreData* miiData, int, u16 index)
{
    *miiData = {};
    int result = RFLGetStoreData(miiData, 0, index);
    ClearMiiInfo(miiData);
    return result;
}

// Clear hidden Mii info from SAKE FriendInfo
WWFC_DEFINE_PATCH = Patch::CallWithCTR(
    WWFC_PATCH_LEVEL_BUGFIX, //
    // RMC:RMCN:Nwc24Manager::executeFriendInfo
    RMCXD_PORT(0x80672E80, 0x8066B71C, 0x806724EC, 0x806611D8, 0x806733C4) //
    RMCXN_PORT(0x801FCF58, 0x801FCEB8, 0x801FCCF0, 0x801FD748), //

    ASM_LAMBDA(
        ( : ASM_IMPORT(i, RFLGetStoreDataOverride)),
        // clang-format off
        lhz     r5, 0x8(r1);
        mr      r3, r31;
        b       %[RFLGetStoreDataOverride];
        // clang-format on
    )
);

// Clear hidden Mii info from ghost data upon upload
WWFC_DEFINE_PATCH = Patch::CallWithCTR(
    WWFC_PATCH_LEVEL_BUGFIX, //
    // RMC:RMCN:Nwc24Manager::uploadGhost
    RMCXD_PORT(0x8066AE60, 0x806636FC, 0x8066A4CC, 0x806591B8, 0x8066B3A4) //
    RMCXN_PORT(0x801F6268, 0x801F61C8, 0x801F6000, 0x801F6A58), //

    ASM_LAMBDA(
        ( : ASM_IMPORT(i, RFLGetStoreDataOverride)),
        // clang-format off
        lhz     r5, 0x8(r1);
        addi    r3, r1, 0x80;
        b       %[RFLGetStoreDataOverride];
        // clang-format on
    )
);

// Clear hidden Mii info from ghost upload playerinfo
WWFC_DEFINE_PATCH = Patch::CallWithCTR(
    WWFC_PATCH_LEVEL_BUGFIX, //
    // RMC:RMCN:Nwc24GdbManager::postGhostData
    RMCXD_PORT(0x80677488, 0x8066FD24, 0x80676AF4, 0x80665830, 0x806779CC) //
    RMCXN_PORT(0x801FF0E0, 0x801FF040, 0x801FEE78, 0x801FF920), //

    [](const mkw::Nwc24::PlayerInfo* info) {
        mkw::Nwc24::PlayerInfo copy = *info;
        ClearMiiInfo(&copy.mii);
    // Such behavior is a little scary:
#  pragma GCC diagnostic push
#  pragma GCC diagnostic ignored "-Wframe-address"
        char* output =
            reinterpret_cast<char*>(__builtin_frame_address(1)) + 0x18;
#  pragma GCC diagnostic pop
        return GameSpy::B64Encode(
            reinterpret_cast<char*>(&copy), output,
            sizeof(mkw::Nwc24::PlayerInfo), 2
        );
    }
);

// Clear hidden Mii info from playerinfobase64
WWFC_DEFINE_PATCH = Patch::CallWithCTR(
    WWFC_PATCH_LEVEL_BUGFIX, //
    // RMC:RMCN:Nwc24GdbManager::executeSubmitScoresSoap
    RMCXD_PORT(0x8067AAE0, 0x80673068, 0x8067A14C, 0x80668E88, 0x8067B024) //
    RMCXN_PORT(0x80201180, 0x802010E0, 0x80200F18, 0x802019C0), //

    [](GameSpy::GSXmlStreamWriter stream, const char*, const char*,
       const mkw::Nwc24::PlayerInfo* data) -> GameSpy::gsi_bool {
        mkw::Nwc24::PlayerInfo copy = *data;
        ClearMiiInfo(&copy.mii);
        return GameSpy::gsXmlWriteBase64BinaryElement(
            stream, "ns1", "playerinfobase64", &copy,
            sizeof(mkw::Nwc24::PlayerInfo)
        );
    }
);

static void ClearMiiInfo(RFL::RFLiStoreData* miiData)
{
    // Mainly just clear the create ID stuff, which you can get the MAC
    // address from
    miiData->data.createID.miiID = 0x80000000;
    miiData->data.createID.consoleID = 0;

    // But while we're here, clear some other stuff too.
    // A flaw in the Mii Channel means that if a Mii name is shortened, the
    // string is only overwritten up to the new string's length, leaving the end
    // of the previous name intact
    bool hitNullTerminator = false;
    for (u32 i = 0; i < RFL::RFL_NAME_LEN; i++) {
        if (hitNullTerminator) {
            miiData->data.name[i] = 0;
        } else if (miiData->data.name[i] == 0) {
            hitNullTerminator = true;
        }
    }

    // Creator names have the same issue, but we just clear them completely
    // because they're not needed
    std::memset(
        miiData->data.creatorName, 0, sizeof(miiData->data.creatorName)
    );

    // Clear birthday
    miiData->data.birthMonth = 0;
    miiData->data.birthDay = 0;

    // Recalculate the CRC16-CCITT checksum
    miiData->checksum = 0;
    miiData->checksum =
        RFL::RFLiCalculateCRC(miiData, sizeof(RFL::RFLiStoreData));
}

} // namespace wwfc::Mii

#endif // RMC || RMCN

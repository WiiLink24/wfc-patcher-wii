#if RMC

#  include "import/mkw/net/userHandler.hpp"
#  include "wwfcGPReport.hpp"
#  include "wwfcLibC.hpp"
#  include "wwfcPatch.hpp"

namespace wwfc::Mii
{

static void ClearUserMiiInfo(mkw::Net::UserHandler::Packet* packet);
static int
RFLGetStoreDataOverride(RFLiStoreData* miiData, int source, u16 index);

WWFC_DEFINE_PATCH = Patch::CallWithCTR(
    WWFC_PATCH_LEVEL_BUGFIX, //
    RMCXD_PORT(0x80663178, 0x80661094, 0x806627E4, 0x80651490, DEMOTODO), //
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

WWFC_DEFINE_PATCH = Patch::CallWithCTR(
    WWFC_PATCH_LEVEL_BUGFIX, //
    RMCXD_PORT(0x80672E80, 0x8066B71C, 0x806724EC, 0x806611D8, DEMOTODO), //
    ASM_LAMBDA(
        ( : ASM_IMPORT(i, RFLGetStoreDataOverride)),
        // clang-format off
        lhz     r5, 0x8(r1);
        mr      r3, r31;
        li      r4, 0;
        b       %[RFLGetStoreDataOverride];
        // clang-format on
    ),
    5
);

static void ClearMiiInfo(RFLiStoreData* miiData)
{
    // Mainly just clear the create ID stuff, which you can get the MAC
    // address from
    miiData->data.createID.miiID = 0x80000000;
    miiData->data.createID.consoleID = 0;

    // But while we're here, clear some other stuff too
    // A flaw in the Mii Channel means that if a Mii name is shortened, the
    // string is only overwritten up to the new string's length, leaving the end
    // of the previous name intact.
    bool hitNullTerminator = false;
    for (u32 i = 0; i < RFL_NAME_LEN; i++) {
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
    [[gnu::longcall]] u16 RFLiCalculateCRC( //
        const void* data, u32 size
    ) AT(RMCXD_PORT(0x800C78D0, 0x800C7830, 0x800C77F0, 0x800C7930, DEMOTODO));

    miiData->checksum = 0;
    miiData->checksum = RFLiCalculateCRC(miiData, sizeof(RFLiStoreData));
}

static void ClearUserMiiInfo(mkw::Net::UserHandler::Packet* packet)
{
    // Clear hidden Mii info
    // Mii Group count _should_ be 2, but could be higher if the code has been
    // messed with
    for (u32 i = 0; i < packet->miiGroupCount; i++) {
        // Check if this is a guest "no name" Mii
        [[gnu::longcall]] bool RFLSearchOfficialData( //
            const RFLCreateID* id, u16* index
        ) AT(RMCXD_PORT(0x800CA820, 0x800CA780, 0x800CA740, 0x800CA880, DEMOTODO));

        u16 discard;
        if (!RFLSearchOfficialData(
                &packet->miiData[i].data.createID, &discard
            )) {
            // It's not a guest Mii
            ClearMiiInfo(&packet->miiData[i]);
        }
    }

    // Send Mii data to the server
    wwfc::GPReport::ReportB64Encode(
        "wl:mkw_user", packet, sizeof(mkw::Net::UserHandler::Packet)
    );
}

static int
RFLGetStoreDataOverride(RFLiStoreData* miiData, int source, u16 index)
{
    [[gnu::longcall]] int RFLGetStoreData( //
        RFLiStoreData* data, int source, u16 index
    ) AT(RMCXD_PORT(0x800C7DF0, 0x800C7D50, 0x800C7D10, 0x800C7E50, DEMOTODO));

    *miiData = {};
    int result = RFLGetStoreData(miiData, source, index);
    ClearMiiInfo(miiData);
    return result;
}

} // namespace wwfc::Mii

#endif

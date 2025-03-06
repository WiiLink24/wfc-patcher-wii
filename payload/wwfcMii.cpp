#include "import/mkw/net/userHandler.hpp"
#include "wwfcGPReport.hpp"
#include "wwfcPatch.hpp"
#include <cstring>

namespace wwfc::Mii
{

#if RMC

void ClearUserMiiInfo(mkw::Net::UserHandler::Packet* packet
) asm("ClearUserMiiInfo");

WWFC_DEFINE_PATCH = {Patch::CallWithCTR(
    WWFC_PATCH_LEVEL_FEATURE, //
    RMCXD_PORT(0x80663178, 0x80661094, 0x806627E4, 0x80651490), //
    ASM_LAMBDA(
        // clang-format off
        mflr    r30;
        addi    r3, r31, 0x8;
        bl      ClearUserMiiInfo;
        mtlr    r30;
        // Function epilogue
        lwz     r31, 0x1C(sp);
        lwz     r30, 0x18(sp);
        lwz     r29, 0x14(sp);
        lwz     r0, 0x24(sp);
        blr;
        // clang-format on
    )
)};

static void ClearMiiInfo(RFLiStoreData* miiData)
{
    // Check if this is a guest "no name" Mii
    LONGCALL bool RFLSearchOfficialData( //
        const RFLCreateID* id, u16* index
    ) AT(RMCXD_PORT(0x800CA820, 0x800CA780, 0x800CA740, 0x800CA880));

    u16 discard;
    if (RFLSearchOfficialData(&miiData->data.createID, &discard)) {
        // It's a guest Mii
        return;
    }

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
    LONGCALL u16 RFLiCalculateCRC( //
        const void* data, u32 size
    ) AT(RMCXD_PORT(0x800C78D0, 0x800C7830, 0x800C77F0, 0x800C7930));

    miiData->checksum = 0;
    miiData->checksum = RFLiCalculateCRC(miiData, sizeof(RFLiStoreData));
}

void ClearUserMiiInfo(mkw::Net::UserHandler::Packet* packet)
{
    // Clear hidden Mii info
    // Mii Group count _should_ be 2, but could be higher if the code has been
    // messed with
    for (u32 i = 0; i < packet->miiGroupCount; i++) {
        ClearMiiInfo(&packet->miiData[i]);
    }

    // Send Mii data to the server
    wwfc::GPReport::ReportB64Encode(
        "wl:mkw_user", packet, sizeof(mkw::Net::UserHandler::Packet)
    );
}

#endif

} // namespace wwfc::Mii

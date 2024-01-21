#include "import/gamespy.h"
#include "import/revolution.h"
#include "wwfcPatch.hpp"

namespace wwfc::Natneg
{

// Remove the pointless 5 second delay (FINISHED_IDLE_TIME) after connection
// completed
#if ADDRESS_NATNEG_SET_COMPLETED_DELAY
WWFC_DEFINE_PATCH = {Patch::WriteASM(
    WWFC_PATCH_LEVEL_BUGFIX | WWFC_PATCH_LEVEL_FEATURE |
        WWFC_PATCH_LEVEL_PARITY, //
    ADDRESS_NATNEG_SET_COMPLETED_DELAY, //
    1, ASM_LAMBDA(addi r0, r3, 0)
)};
#endif

s32 GT2CreateSocket(
    void* sock, const char* localAddress, int outgoingBufferSize,
    int incomingBufferSize, void* callback
) asm("GT2CreateSocket");

// Use a specific port for GT2/QR2/NATNEG
WWFC_DEFINE_PATCH = {Patch::CallWithCTR(
    WWFC_PATCH_LEVEL_FEATURE, //
    ADDRESS_GT2_PORT_PATCH, //
    ASM_LAMBDA(
        // clang-format off
        lwz     r5, 0x14(GT2_PORT_PATCH_REG);
        lwz     r6, 0x18(GT2_PORT_PATCH_REG);
        mflr    GT2_PORT_PATCH_REG - 1;
        bl      GT2CreateSocket;
        mtlr    GT2_PORT_PATCH_REG - 1;
        mr      GT2_PORT_PATCH_REG - 1, r3;
        blr;
        // clang-format on
    )
)};

s32 GT2CreateSocket(
    void* sock, const char* localAddress, int outgoingBufferSize,
    int incomingBufferSize, void* callback
)
{
    // Get console-specific port, matches the one Wiimmfi uses:
    // 22000 + last three digits of console serial number (including the digit
    // in the square)
    u32 serial;
    if (RVL::SCGetProductSN(&serial)) {
        const u32 port = 22000 + (serial % 1000);
        char addressString[22];
        s32 ret = GameSpy::gt2CreateSocket(
            sock, GameSpy::gt2AddressToString(0, port, addressString),
            outgoingBufferSize, incomingBufferSize, callback
        );
        if (ret != GameSpy::GT2NetworkError) {
            return ret;
        }
    }

    // Fall back to the original function on bind error
    return GameSpy::gt2CreateSocket(
        sock, localAddress, outgoingBufferSize, incomingBufferSize, callback
    );
}

} // namespace wwfc::Natneg
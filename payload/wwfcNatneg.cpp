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

#if RMC
// Attempt to fix the "suspend bug," where DWC stalls suspending the match due
// to ongoing NATNEG between clients. This patch will ignore NATNEG and suspend
// anyway if not the host.
// Credit: MrBean35000vr
WWFC_DEFINE_PATCH = {Patch::CallWithCTR(
    WWFC_PATCH_LEVEL_BUGFIX | WWFC_PATCH_LEVEL_PARITY, //
    RMCXD_PORT(0x800E77F4, 0x800E7754, 0x800E7714, 0x800E7854), //
    ASM_LAMBDA(
        // clang-format off
        lbz     r0, 0x16(r31); // stpMatchCnt->hostState
        cmpwi   r0, 0; // Not in a match
        beqlr; // r0 = 0 will already fall through to 0x800E7814

        cmpwi   r0, 1; // Client in room
        beqlr; // r0 = 1 will already fall through to 0x800E781C

        lwz     r0, 0x71C(r31); // stpMatchCnt->state
        blr;
        // clang-format on
    )
)};
#endif // RMC

#if RSB
// Extend the latency limit in random matchmaking from 100 ms, due to the lack
// of players sometimes preventing matchmaking completely.
WWFC_DEFINE_PATCH = {Patch::WriteASM(
    WWFC_PATCH_LEVEL_PARITY, //
    RSBX_PORT(0x80149268, 0x801492E0, 0x8014A87C, 0x8014A89C), //
    1, ASM_LAMBDA(li r3, 600) // 600 ms
)};
#endif

} // namespace wwfc::Natneg
#if RMC

#  include "import/mkw/net/net.hpp"
#  include "import/mkw/system/raceConfig.hpp"
#  include "wwfcPatch.hpp"
#  include "wwfcPayload.hpp"

namespace wwfc::mkw::BugFix
{

// Prevent forced disconnection from circumstances such as a player using "No
// Countdown 4.1" or loading a save state mid-race.
// Credit: acaruso
WWFC_DEFINE_PATCH = Patch::WriteASM(
    WWFC_PATCH_LEVEL_BUGFIX,
    RMCXD_PORT(0x80655578, 0x806510F0, 0x80654BE4, 0x80643890, 0x80655ABC), //
    1, ASM_LAMBDA((), nop)
);

// Patch the Mario Kart Wii Ultra Shortcut bug
// Credit: MrBean35000vr, Chadderz

static u64 IsUltraShortcutCheckEnabled( //
    u32 r3Discard, u32 r4Save
);

// Patch in RaceManagerPlayer::updateCheckpoint.
// This patch is implemented weirdly to be compatible with the existing
// Ultra UnCut code.
WWFC_DEFINE_PATCH = Patch::CallWithCTR(
    WWFC_PATCH_LEVEL_BUGFIX | WWFC_PATCH_LEVEL_PARITY, //
    RMCXD_PORT(0x80535120, 0x805305D8, 0x80534AA0, 0x80523178, 0x8053493C), //
    ASM_LAMBDA(
        ( : ASM_IMPORT(i, IsUltraShortcutCheckEnabled)),
        // clang-format off

        mflr    r28;
        beq     L%=Equal;

        bl      %[IsUltraShortcutCheckEnabled];
        mtlr    r28;
        // Restore potentially clobbered registers
        mulli   r0, r4, 0x18;
        add     r5, r29, r0; // Restore r5
        lhz     r6, 0x4(r29);  // Restore r6
        
        cmpwi   r3, 0;
        beq     L%=NotEqual;

        lbz     r3, 0x1C(r29);
        cmplwi  r3, 1;
        bgt     L%=Equal;

    L%=NotEqual:;
        // Restore CTR
        sub     r12, r6, r4;
        mtctr   r12; 

        addi    r5, r5, 0x18;
        // Jump to 0x80535130
        blr;

    L%=Equal:;
        // Restore CTR
        sub     r12, r6, r4;
        mtctr   r12; 

        li      r0, 1;
        // Jump to 0x8053513C
        addi    r12, r28, 0xC;
        mtlr    r12;
        blr;
        // clang-format on
    )
);

static u64 IsUltraShortcutCheckEnabled(u32 r3Discard, u32 r4Save)
{
    bool enabled = false;

    auto raceConfig = mkw::System::RaceConfig::Instance();
    if (raceConfig->raceScenario().isOnlineVersusRace()) {
        if (Payload::g_enableUltraUncut == WWFC_BOOLEAN_RESET) {
            // Check if Worldwide or other vanilla match
            auto netController = mkw::Net::NetController::Instance();
            if (netController && netController->inVanillaMatch()) {
                enabled = true;
            }
        } else {
            enabled = Payload::g_enableUltraUncut != WWFC_BOOLEAN_FALSE;
        }
    }

    // Return hack to save r4 from caller
    return (u64(enabled ? 1 : 0) << 32) | r4Save;
}

} // namespace wwfc::mkw::BugFix

#endif // RMC
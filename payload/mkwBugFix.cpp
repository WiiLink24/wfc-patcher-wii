#if RMC

#  include "import/mkw/net/net.hpp"
#  include "import/mkw/system/raceConfig.hpp"
#  include "wwfcPatch.hpp"

namespace wwfc::BugFix
{

u64 IsUltraShortcutCheckEnabled( //
    u32 r3Discard, u32 r4Save
) asm("IsUltraShortcutCheckEnabled");

// Patch the Mario Kart Wii Ultra Shortcut bug
// Credit: MrBean35000vr, Chadderz
WWFC_DEFINE_PATCH = {
    // Patch in RaceManagerPlayer::updateCheckpoint.
    // This patch is implemented weirdly to be compatible with the existing
    // Ultra UnCut code.
    Patch::CallWithCTR(
        WWFC_PATCH_LEVEL_BUGFIX | WWFC_PATCH_LEVEL_PARITY, //
        RMCXD_PORT(0x80535120, 0x805305D8, 0x80534AA0, 0x80523178), //
        ASM_LAMBDA(
            // clang-format off
            beq     L_Equal;

            mflr    r28;
            bl      IsUltraShortcutCheckEnabled;
            mtlr    r28;
            // Restore potentially clobbered registers
            mulli   r0, r4, 0x18;
            add     r5, r29, r0; // Restore r5
            lhz     r6, 0x4(r29);  // Restore r6
        
            cmpwi   r3, 0;
            beq     L_NotEqual;

            lbz     r3, 0x1C(r29);
            cmplwi  r3, 1;
            bgt     L_Equal;

        L_NotEqual:;
            // Restore CTR
            sub     r12, r6, r4;
            mtctr   r12; 

            addi    r5, r5, 0x18;
            // Jump to 0x80535130
            blr;

        L_Equal:;
            // Restore CTR
            sub     r12, r6, r4;
            mtctr   r12; 

            li      r0, 1;
            // Jump to 0x8053513C
            mflr    r12;
            addi    r12, r12, 0xC;
            mtlr    r12;
            blr;
            // clang-format on
        )
    )
};

u64 IsUltraShortcutCheckEnabled(u32 r3Discard, u32 r4Save)
{
    bool enabled = false;

    auto raceConfig = mkw::System::RaceConfig::Instance();
    if (raceConfig->raceScenario().isOnlineVersusRace()) {
        // Check if Worldwide or other vanilla match
        auto netController = mkw::Net::NetController::Instance();
        if (netController && netController->inVanillaMatch()) {
            enabled = true;
        }
    }

    // Return hack to save r4 from caller
    return (u64(enabled ? 1 : 0) << 32) | r4Save;
}

} // namespace wwfc::BugFix

#endif // RMC
#if RMC // Mario Kart Wii only for now

#  include "mkwTime.hpp"
#  include "wwfcPatch.hpp"

namespace wwfc::Process
{

/**
 * Tick function that is called every frame, or whenever
 * DWC_ProcessFriendsMatch() is called.
 */
void Tick()
{
    mkw::Time::Tick();
}

// Call the tick function every frame
WWFC_DEFINE_PATCH = Patch::BranchWithCTR(
    WWFC_PATCH_LEVEL_BUGFIX | WWFC_PATCH_LEVEL_PARITY,
    RMCXD_PORT(0x800D13E8, 0x800D1348, 0x800D1308, 0x800D1448, 0x800D11B8), //
    ASM_LAMBDA(
        ( : ASM_IMPORT(i, Tick)),
        // clang-format off
        lwz     r0, 0x14(r1);
        mtlr    r0;
        addi    r1, r1, 0x10;
        b       %[Tick];
        // clang-format on
    )
);

} // namespace wwfc::Process

#endif
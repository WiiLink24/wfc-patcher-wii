#include "wwfcPatch.hpp"

namespace wwfc::AntiFreeze {

#if RMC

// Prevent invalid profile identifiers from crashing the game
WWFC_DEFINE_PATCH = {
    Patch::WriteASM(
        WWFC_PATCH_LEVEL_BUGFIX,
        RMCXD_PORT(0x805D2EF8, 0x805C5DDC, 0x805D27D4, 0x805C1094),
        1, ASM_LAMBDA(b -0x3C)
    ),
};
WWFC_DEFINE_PATCH = {
    Patch::WriteASM(
        WWFC_PATCH_LEVEL_BUGFIX,
        RMCXD_PORT(0x805D2F00, 0x805C5DE4, 0x805D27DC, 0x805C109C),
        1, ASM_LAMBDA(b -0x44)
    ),
};

#endif

} // wwfc::AntiFreeze

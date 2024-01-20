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

} // namespace wwfc::Natneg
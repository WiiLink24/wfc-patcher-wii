#include "import/mkw/animation.hpp"
#include "import/mkw/system/courseMap.hpp"
#include "wwfcPatch.hpp"

namespace wwfc::AntiFreeze
{

#if RMC

using namespace mkw::System;

static const MapdataItemPoint::Data s_mapdataItemPointData = {
    EGG::Vector3f{0.0f, 0.0f, 0.0f},
    0.0f,
    {0, 0},
};
static MapdataItemPoint s_mapdataItemPoint(&s_mapdataItemPointData);

// Prevent the game from crashing if a Bullet Bill is used during a Battle
WWFC_DEFINE_PATCH = {
    Patch::WriteASM(
        WWFC_PATCH_LEVEL_BUGFIX,
        RMCXD_PORT(0x80514D58, 0x805108E4, 0x805146D8, 0x80502D78), //
        1, ASM_LAMBDA(bge + 0x20)
    ),
    Patch::BranchWithCTR(
        WWFC_PATCH_LEVEL_BUGFIX,
        RMCXD_PORT(0x80514D78, 0x80510904, 0x805146F8, 0x80502D98), //
        [](void* /* mapdataItemPointAccessor */
        ) -> MapdataItemPoint* { return &s_mapdataItemPoint; }
    ),
};

static const MapdataCannonPoint::Data s_mapdataCannonPointData = {
    EGG::Vector3f{0.0f, 0.0f, 0.0f},
    EGG::Vector3f{0.0f, 0.0f, 0.0f},
    0,
    MapdataCannonPoint::CannonType::Default,
};
static MapdataCannonPoint s_mapdataCannonPoint(&s_mapdataCannonPointData);

// Prevent the game from crashing if a cannon is entered on a course without one
WWFC_DEFINE_PATCH = {
    Patch::WriteASM(
        WWFC_PATCH_LEVEL_BUGFIX,
        RMCXD_PORT(0x80518AFC, 0x80514688, 0x8051847C, 0x80506B1C), //
        1, ASM_LAMBDA(bge + 0x20)
    ),
    Patch::BranchWithCTR(
        WWFC_PATCH_LEVEL_BUGFIX,
        RMCXD_PORT(0x80518B1C, 0x805146A8, 0x8051849C, 0x80506B3C), //
        [](void* /* mapdataCannonPointAccessor */
        ) -> MapdataCannonPoint* { return &s_mapdataCannonPoint; }
    ),
};

// Prevent invalid profile identifiers from crashing the game
WWFC_DEFINE_PATCH = {
    Patch::WriteASM(
        WWFC_PATCH_LEVEL_BUGFIX,
        RMCXD_PORT(0x805D2EF8, 0x805C5DDC, 0x805D27D4, 0x805C1094), //
        1, ASM_LAMBDA(b - 0x3C)
    ),
    Patch::WriteASM(
        WWFC_PATCH_LEVEL_BUGFIX,
        RMCXD_PORT(0x805D2F00, 0x805C5DE4, 0x805D27DC, 0x805C109C), //
        1, ASM_LAMBDA(b - 0x44)
    ),
};

// Prevent the game from crashing if a Thwomp is damaged before it touches the
// ground
WWFC_DEFINE_PATCH = {
    Patch::CallWithCTR(
        WWFC_PATCH_LEVEL_BUGFIX,
        RMCXD_PORT(0x80760A88, 0x80753B3C, 0x807600F4, 0x8074EE48), //
        // clang-format off
        [](mkw::Model::AnimationTexturePattern* animationTexturePattern
        ) -> float {
            if (!animationTexturePattern) {
                return 0.0f;
            }

            return animationTexturePattern->GetFrameCount();
        }
        // clang-format on
    ),
};

#endif

} // namespace wwfc::AntiFreeze

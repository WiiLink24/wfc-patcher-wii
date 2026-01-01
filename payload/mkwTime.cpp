#if RMC

#  include "import/mkw/system/raceConfig.hpp"
#  include "import/mkw/system/raceManager.hpp"
#  include "import/revolution.h"
#  include "wwfcHostPlatform.hpp"
#  include "wwfcLog.hpp"
#  include "wwfcPatch.hpp"
#  include "wwfcTypes.h"

namespace wwfc::mkw::Time
{

static constinit s32 s_raceStartMs = 0;

constexpr u32 MsecToFrames(u32 ms)
{
    constexpr u64 msToFrames = (60ull << 32) / 1000;

    return (u32((u64(ms) * msToFrames) >> 32));
}

constexpr u32 FramesToMsec(u32 frames)
{
    constexpr u64 framesToMs = (1000ull << 32) / 60;

    return (u32((u64(frames) * framesToMs) >> 32));
}

constexpr s32 MsecRoundFrames(s32 ms)
{
    if (ms < 0) {
        return -MsecRoundFrames(-ms);
    }
    return FramesToMsec(MsecToFrames(ms));
}

constexpr s32 CompareTimeBaseMs(u64 tb, u32 ms)
{
    constexpr u64 msToTb = 4000ull;

    return (tb - u64(ms) * msToTb) / msToTb;
}

static_assert(MsecRoundFrames(34) == 33);

bool GetElapsedMsec(s32& ms)
{
    if (HostPlatform::g_dolphinFd < 0) {
        u32 hi, lo, hi2;
        do {
            asm volatile("mftbu %0; mftbl %1; mftbu %2"
                         : "=r"(hi), "=r"(lo), "=r"(hi2));
        } while (hi != hi2);
        ms = CompareTimeBaseMs((u64(hi) << 32) | lo, ms);
        return true;
    } else {
        alignas(32) RVL::IOVector vectors[1];
        alignas(32) u32 ms2 = 0;
        vectors[0].data = &ms2;
        vectors[0].size = sizeof(ms2);
        if (RVL::IOS_Ioctlv(HostPlatform::g_dolphinFd, 0x01, 0, 1, vectors)) {
            return false;
        }
        ms = ms2 - ms;
        return true;
    }
}

static void FixRaceFinishTime(mkw::System::RaceManager::Player& player)
{
    if (!mkw::System::RaceConfig::Instance()
             ->raceScenario()
             .isOnlineVersusRace()) {
        return;
    }

    s32 ms = s_raceStartMs;
    if (!GetElapsedMsec(ms)) {
        return;
    }
    ms = MsecRoundFrames(ms);

    mkw::System::Timer::Time& time = *player.m_raceFinishTime;

    u32 ingameMs =
        time.m_milliseconds + time.m_seconds * 1000 + time.m_minutes * 60000;

    WWFC_LOG_INFO_FMT("Time (ms) difference: %d", ms - ingameMs);

    if (ms - ingameMs > 83 || ms - ingameMs < -83) {
        // If more than 5 frames difference, set the finish time to the real
        // world time.
        time.m_minutes = ms / 60000;
        ms -= time.m_minutes * 60000;
        time.m_seconds = ms / 1000;
        ms -= time.m_seconds * 1000;
        time.m_milliseconds = ms;
    }
}

WWFC_DEFINE_CTR_STUB( //
    RMCXD_PORT(0x80531F80, 0x8052D438, 0x80531900, 0x8051FFD8, DEMOTODO), //
    u32 RaceConfig_loadNextCourse(mkw::System::RaceConfig* raceConfig),

    // clang-format off
    lwz     r0, 0xB70(r3);
    cmpwi   r0, 0;
    bnelr;
    lwz     r4, 0xB8C(r3);
    // clang-format on
)

WWFC_DEFINE_PATCH = Patch::BranchWithCTR(
    WWFC_PATCH_LEVEL_PARITY | WWFC_PATCH_LEVEL_BUGFIX, //
    RMCXD_PORT(0x80531F80, 0x8052D438, 0x80531900, 0x8051FFD8, DEMOTODO), //
    [](mkw::System::RaceConfig* raceConfig) -> void {
    if (raceConfig->raceScenario().isOnlineVersusRace()) {
        s_raceStartMs = 0;
        GetElapsedMsec(s_raceStartMs);
    }
    RaceConfig_loadNextCourse(raceConfig);
}
);

WWFC_DEFINE_PATCH = Patch::CallWithCTR(
    WWFC_PATCH_LEVEL_PARITY | WWFC_PATCH_LEVEL_BUGFIX, //
    RMCXD_PORT(0x80534920, 0x8052FDD8, 0x805342A0, 0x80522978, DEMOTODO), //
    ASM_LAMBDA(
        ( : ASM_IMPORT(i, FixRaceFinishTime)),
        // clang-format off
        mr      r3, r31;
        mflr    r31;
        bl      %[FixRaceFinishTime];
        mtctr   r31;
        lwz     r0, 0x14(r1);
        lwz     r31, 0xC(r1);
        mtlr    r0;
        addi    r1, r1, 0x10;
        bctr; // Jump back to blr in case of a tail hook
        // clang-format on
    )
);

} // namespace wwfc::mkw::Time

#endif // RMC

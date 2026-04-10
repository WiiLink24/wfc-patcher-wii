
#if RMC

#  include "import/mkw/system/raceConfig.hpp"
#  include "import/mkw/system/raceManager.hpp"
#  include "import/revolution.h"
#  include "wwfcGPReport.hpp"
#  include "wwfcHostPlatform.hpp"
#  include "wwfcPatch.hpp"
#  include "wwfcTypes.h"

namespace wwfc::mkw::Time
{

namespace
{

constexpr u64 MS_TO_TB = 4000ull;

constinit u64 s_raceStartMs = 0;

struct FinishTimeReport {
    u32 inGameTime;
    volatile u32 finishTime;
    u32 difference;
} static constinit s_finishTimeReport = {};

u64 GetTimebase()
{
    u32 hi, lo, hi2;
    do {
        asm volatile("mftbu %0; mftbl %1; mftbu %2"
                     : "=r"(hi), "=r"(lo), "=r"(hi2));
    } while (hi != hi2);
    return (static_cast<u64>(hi) << 32) | lo;
}

constexpr u32 MsecToFrames(u32 ms)
{
    constexpr u64 msToFrames = (60ull << 32) / 1000;

    return static_cast<u32>((static_cast<u64>(ms) * msToFrames) >> 32);
}

constexpr u32 FramesToMsec(u32 frames)
{
    constexpr u64 framesToMs = (1000ull << 32) / 60;

    return static_cast<u32>((static_cast<u64>(frames) * framesToMs) >> 32);
}

constexpr u32 MsecRoundFrames(u32 ms)
{
    return FramesToMsec(MsecToFrames(ms));
}

constexpr u64 CompareTimeBaseMs(u64 tb, u64 ms)
{
    return (tb - static_cast<u64>(ms) * MS_TO_TB) / MS_TO_TB;
}

std::optional<u64> GetElapsedMsec(u64 ms)
{
    if (HostPlatform::g_dolphinFd < 0) {
        return CompareTimeBaseMs(GetTimebase(), ms);
    }

    alignas(32) RVL::IOVector vectors[1];
    alignas(32) u32 ms2 = 0;
    vectors[0].data = &ms2;
    vectors[0].size = sizeof(ms2);
    if (RVL::IOS_Ioctlv(HostPlatform::g_dolphinFd, 0x01, 0, 1, vectors)) {
        return std::nullopt;
    }
    return ms2 - static_cast<u32>(ms);
}

void FixRaceFinishTime(System::RaceManager::Player& player)
{
    if (auto& scenario = System::RaceConfig::Instance()->raceScenario();
        !scenario.isOnlineVersusRace() ||
        scenario.getPlayer(player.m_id)->m_type !=
            System::RaceConfig::Player::Type::Master) {
        return;
    }

    auto result = GetElapsedMsec(s_raceStartMs);
    if (!result.has_value()) {
        return;
    }
    u64 ms = MsecRoundFrames(static_cast<u32>(*result));

    System::Time& time = System::RaceManager::Instance()->m_timer->m_time[0];
    u32 inGameMs =
        time.m_milliseconds + time.m_seconds * 1000 + time.m_minutes * 60000;
    System::Time& finishTime = *player.m_raceFinishTime;
    u32 finishTimeMs = finishTime.m_milliseconds + finishTime.m_seconds * 1000 +
                       finishTime.m_minutes * 60000;
    u32 difference = ms - inGameMs;

    if (s32(difference) > 83) {
        // If more than 5 frames difference, add the difference to the finish
        // time
        ms = finishTimeMs + difference;
        finishTime.m_minutes = ms / 60000;
        ms -= finishTime.m_minutes * 60000;
        finishTime.m_seconds = ms / 1000;
        ms -= finishTime.m_seconds * 1000;
        finishTime.m_milliseconds = ms;
    }

    s_finishTimeReport.inGameTime = inGameMs;
    s_finishTimeReport.difference = difference;
    s_finishTimeReport.finishTime = finishTimeMs;
}

} // namespace

WWFC_DEFINE_CTR_STUB_SAVE_LR( //
    RMCXD_PORT(0x80531F80, 0x8052D438, 0x80531900, 0x8051FFD8, 0x8053179C) +
        0x10, //
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
    RMCXD_PORT(0x80531F80, 0x8052D438, 0x80531900, 0x8051FFD8, 0x8053179C),

    [](mkw::System::RaceConfig* raceConfig) -> void {
        s_raceStartMs = 0;
        if (raceConfig->raceScenario().isOnlineVersusRace()) {
            GetElapsedMsec(s_raceStartMs);
        }
        RaceConfig_loadNextCourse(raceConfig);
    }
);

WWFC_DEFINE_PATCH = Patch::CallWithCTR(
    WWFC_PATCH_LEVEL_PARITY | WWFC_PATCH_LEVEL_BUGFIX, //
    RMCXD_PORT(0x80534920, 0x8052FDD8, 0x805342A0, 0x80522978, 0x8053413C),

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

void Tick()
{
    if (s_finishTimeReport.finishTime == 0) {
        return;
    }
    FinishTimeReport report = s_finishTimeReport;
    s_finishTimeReport.finishTime = 0;

    GPReport::ReportB64Encode("wl:mkw_finish_time", &report, sizeof(report));
}

} // namespace wwfc::mkw::Time

#endif // RMC

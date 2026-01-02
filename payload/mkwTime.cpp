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

static constinit u32 s_raceStartMs = 0;

struct FixedTime {
    /* 0x0 */ u16 m_minutes;
    /* 0x2 */ u8 m_seconds;
    /* 0x3 */ u8 m_reported;
    /* 0x4 */ u16 m_milliseconds;
};

static constinit FixedTime s_raceEndTimes[4] = {};

static constexpr u32 MsecToFrames(u32 ms)
{
    constexpr u64 msToFrames = (60ull << 32) / 1000;

    return (u32((u64(ms) * msToFrames) >> 32));
}

static constexpr u32 FramesToMsec(u32 frames)
{
    constexpr u64 framesToMs = (1000ull << 32) / 60;

    return (u32((u64(frames) * framesToMs) >> 32));
}

static constexpr u32 MsecRoundFrames(u32 ms)
{
    return FramesToMsec(MsecToFrames(ms));
}

static constexpr u32 CompareTimeBaseMs(u64 tb, u32 ms)
{
    constexpr u64 msToTb = 4000ull;

    return (tb - u64(ms) * msToTb) / msToTb;
}

static_assert(MsecRoundFrames(34) == 33);

static bool GetElapsedMsec(u32& ms)
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
    System::RaceConfig::Scenario& scenario =
        mkw::System::RaceConfig::Instance()->raceScenario();

    if (!scenario.isOnlineVersusRace()) {
        return;
    }

    System::RaceConfig::Player& playerConfig = *scenario.getPlayer(player.m_id);
    if (playerConfig.m_playerType !=
            System::RaceConfig::Player::PlayerType::Master ||
        playerConfig.m_localPlayerId == 0xFF) {
        return;
    }

    u32 ms = s_raceStartMs;
    if (!GetElapsedMsec(ms)) {
        return;
    }
    ms = MsecRoundFrames(ms);

    System::Timer::Time& time = *player.m_raceFinishTime;

    u32 ingameMs =
        time.m_milliseconds + time.m_seconds * 1000 + time.m_minutes * 60000;

    WWFC_LOG_INFO_FMT("Time (ms) difference: %d", ms - ingameMs);

    if (s32(ms - ingameMs) > 83) {
        // If more than 5 frames difference, and if not negative, set the finish
        // time to the real world time.
        FixedTime& fixedTime = s_raceEndTimes[playerConfig.m_localPlayerId];
        if (ms >= 60000 * 63) {
            // Clamp to 62:59.999
            fixedTime = {
                .m_minutes = 62,
                .m_seconds = 59,
                .m_reported = 1,
                .m_milliseconds = 999,
            };
            return;
        }

        fixedTime = {
            .m_minutes = u16(ms / 60000),
            .m_seconds = u8((ms / 1000) % 60),
            .m_reported = 1,
            .m_milliseconds = u16(ms % 1000),
        };
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
        std::memset(s_raceEndTimes, 0, sizeof(s_raceEndTimes));
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

// Send the fixed finish time to remote players
WWFC_DEFINE_PATCH = Patch::CallWithCTR(
    WWFC_PATCH_LEVEL_BUGFIX | WWFC_PATCH_LEVEL_PARITY, //
    RMCXD_PORT(0x8053E9F8, 0x805394BC, 0x8053E378, 0x8052CA50, DEMOTODO), //
    ASM_LAMBDA(
        ( : ASM_IMPORT(i, s_raceEndTimes)),
        // clang-format off
        mflr    r12;
        bcl     20, 31, (+8);
    L%=RaceEndTimesPtr:;
        .long   (%[s_raceEndTimes]-4)@fixup;
        mflr    r7;
        mtlr    r12;
        lwz     r7, 0(r7);
        mulli   r6, r26, 0x6;
        add     r7, r7, r6;

        lbz     r6, 4+0x3(r7); // m_reported
        cmpwi   cr5, r6, 0;
        beq-    cr5, L%=LoadNormalTime;

        mr      r3, r7;
        b       L%=LoadNormalTime+4;

    L%=LoadNormalTime:;
        lbz     r6, 0xA(r3); // m_valid
        lhz     r9, 0x4(r3); // m_minutes
        lbz     r8, 0x6(r3); // m_seconds
        lhz     r7, 0x8(r3); // m_milliseconds
        blr;

        .pushsection ".fixup", "aw";
        .align  2;
        .long   L%=RaceEndTimesPtr@fixup;
        .popsection;
        // clang-format on
    )
);

} // namespace wwfc::mkw::Time

#endif // RMC

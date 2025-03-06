#pragma once

#include "import/mkw/util.hpp"
#include "net.hpp"

namespace mkw::Net
{

#if RMC

// https://github.com/SeekyCt/mkw-structures/blob/master/selecthandler.h
class SelectHandler
{
public:
    struct __attribute__((packed)) Packet {
        enum class SelectedCourse : u8 {
            NotSelected = 0xFF,
        };

        enum class Phase : u8 {
            Preparing = 0,
            Voting = 1,
            Lottery = 2,
        };

        enum class EngineClass : u8 {
            e100cc = 1,
            e150cc = 2,
            eMirrorMode = 3,
        };

        struct Player {
            enum class Character : u8 {
                NotSelected = 0x30,
            };

            enum class Vehicle : u8 {
                NotSelected = 0x24,
            };

            enum class CourseVote : u8 {
                NotSelected = 0x43,
                Random = 0xFF,
            };

            /* 0x00 */ u8 _00[0x04 - 0x00];
            /* 0x04 */ Character character;
            /* 0x05 */ Vehicle vehicle;
            /* 0x06 */ CourseVote courseVote;
            /* 0x07 */ u8 _07;
        };

        static_assert(sizeof(Player) == 0x08);

        /* 0x00 */ u8 _00[0x10 - 0x00];
        /* 0x10 */ Player player[2];
        /* 0x20 */ u8 _20[0x34 - 0x20];
        /* 0x34 */ SelectedCourse selectedCourse;
        /* 0x35 */ Phase phase;
        /* 0x36 */ u8 _36;
        /* 0x37 */ EngineClass engineClass;
    };

    static_assert(sizeof(Packet) == 0x38);

    Packet& sendPacket()
    {
        return m_sendPacket;
    }

    void decideCourse()
    {
        LONGCALL void decideCourse(SelectHandler * selectHandler)
            AT(RMCXD_PORT(0x80661CE8, 0x80659DAC, 0x80661354, 0x80650000));

        decideCourse(this);
    }

    void initPlayerIdsToPlayerAids()
    {
        LONGCALL void initPlayerIdsToPlayerAids(SelectHandler * selectHandler)
            AT(RMCXD_PORT(0x80662034, 0x8065A0F8, 0x806616A0, 0x8065034C));

        initPlayerIdsToPlayerAids(this);
    }

    // https://github.com/CLF78/OpenPayload/blob/master/payload/wiimmfi/RoomStall.cpp
    void processKicks()
    {
        NetController* netController = NetController::Instance();
        if (!netController->amITheServer()) {
            return;
        }

        if (s_kickTimerFrames < s_kickTimerThresholdFrames) {
            s_kickTimerFrames++;

            return;
        }
        s_kickTimerFrames = 0;

        u32 availableAids =
            netController->availableAids() & ~(1 << netController->myAid());

        u32 aidsStillLoading = 0x00000000;
        switch (m_sendPacket.phase) {
        case Packet::Phase::Preparing: {
            aidsStillLoading = (~m_aidsWithNewSelectPacket) & availableAids;
            if (aidsStillLoading) {
                break;
            }

            aidsStillLoading = (~m_aidsWithNewMatchSettings) & availableAids;
            break;
        }
        case Packet::Phase::Voting: {
            aidsStillLoading = (~m_aidsWithVote) & availableAids;
            if (aidsStillLoading) {
                break;
            }

            aidsStillLoading = (~m_aidsWithVoteData) & availableAids;
            break;
        }
        default: {
            return;
        }
        }

        // Support modifications that allow for clients to be connected to more
        // than 11 peers at once.
        for (size_t n = 0; n < sizeof(aidsStillLoading) * 8; n++) {
            if (((aidsStillLoading >> n) & 1) == 0) {
                continue;
            }

            netController->reportAndKick("wl:stall", n);
        }
    }

    static SelectHandler* Instance()
    {
        return s_instance;
    }

    static void ResetKickTimer()
    {
        s_kickTimerFrames = 0;
    }

private:
    /* 0x000 */ u8 _000[0x008 - 0x000];
    /* 0x008 */ Packet m_sendPacket;
    /* 0x040 */ u8 _040[0x3E0 - 0x040];
    /* 0x3E0 */ u32 m_aidsWithNewSelectPacket;
    /* 0x3E4 */ u8 _3E4[0x3E8 - 0x3E4];
    /* 0x3E8 */ u32 m_aidsWithNewMatchSettings;
    /* 0x3EC */ u32 m_aidsWithVoteData;
    /* 0x3F0 */ u32 m_aidsWithVote;
    /* 0x3F4 */ u8 _3F4[0x3F8 - 0x3F4];

    static u32 s_kickTimerFrames;

    static constexpr u32 s_kickTimerThresholdFrames = 90 * 60;

    static SelectHandler* s_instance
        AT(RMCXD_PORT(0x809C2100, 0x809BD930, 0x809C1160, 0x809B0740));
};

static_assert(sizeof(SelectHandler) == 0x3F8);

#endif

} // namespace mkw::Net

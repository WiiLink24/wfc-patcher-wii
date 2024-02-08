#pragma once

#include "mkwHostSystem.hpp"

namespace mkw::Net
{

#if RMC

struct RacePacket {
    enum EType {
        Header,
        MatchHeader,
        MatchData,
        RoomSelect,
        PlayerData,
        User,
        Item,
        Event,
    };

    /* 0x00 */ u32 pad;
    /* 0x04 */ u32 checksum;
    /* 0x08 */ u8 sizes[8];
};

static_assert(sizeof(RacePacket) == 0x10);

// https://github.com/SeekyCt/mkw-structures/blob/master/rknetcontroller.h
class RKNetController
{
public:
    enum JoinType {
        NotJoining = 0,
        WorldwideVersusRace = 1,
        ContinentalVersusRace = 2,
        WorldwideBattle = 3,
        ContinentalBattle = 4,
        RoomHost = 5,
        RoomGuest = 6,
        FriendWorldwideVersusRace = 7,
        FriendContinentalVersusRace = 8,
        FriendWorldwideBattle = 9,
        FriendContinentalBattle = 10,
    };

    struct ConnectionInfo {
        /* 0x00 */ u8 _00[0x21 - 0x00];
        /* 0x21 */ u8 myAid;
        /* 0x22 */ u8 serverAid;
        /* 0x23 */ u8 _23[0x58 - 0x23];
    };

    static_assert(sizeof(ConnectionInfo) == 0x58);

    void
    processRacePacket(u32 playerAid, RacePacket* racePacket, u32 packetSize)
    {
        LONGCALL void processRacePacket(
            RKNetController * rkNetController, u32 playerAid,
            RacePacket * racePacket, u32 packetSize
        ) AT(RMCXD_PORT(0x80659A84, 0x806555FC, 0x806590F0, 0x80647D9C));

        processRacePacket(this, playerAid, racePacket, packetSize);
    }

    ConnectionInfo& currentConnectionInfo()
    {
        return m_connectionInfo[m_currentConnectionInfoIndex];
    }

    JoinType joinType() const
    {
        return m_joinType;
    }

    bool inVanillaMatch() const
    {
        switch (m_joinType) {
        case JoinType::WorldwideVersusRace:
        case JoinType::WorldwideBattle:
        case JoinType::FriendWorldwideVersusRace:
        case JoinType::FriendWorldwideBattle: {
            return true;
        }
        case JoinType::ContinentalVersusRace:
        case JoinType::ContinentalBattle:
        case JoinType::FriendContinentalVersusRace:
        case JoinType::FriendContinentalBattle: {
            using namespace mkw::HostSystem;

            // Allow clients to modify their region without having to change
            // their matching area.
            extern SystemManager::MatchingArea customMatchingArea AT(0x80005EFC
            );
            if (customMatchingArea < SystemManager::MatchingArea::Japan ||
                customMatchingArea > SystemManager::MatchingArea::China) {
                return false;
            }

            SystemManager::MatchingArea matchingArea =
                SystemManager::Instance()->matchingArea();
            return matchingArea >= SystemManager::MatchingArea::Japan &&
                   matchingArea <= SystemManager::MatchingArea::China;
        }
        case JoinType::RoomHost:
        case JoinType::RoomGuest: {
            return false;
        }
        default:
            return false;
        }
    }

    static RKNetController* Instance()
    {
        return s_instance;
    }

private:
    /* 0x0000 */ u8 _0000[0x0038 - 0x0000];
    /* 0x0038 */ ConnectionInfo m_connectionInfo[2];
    /* 0x00E8 */ JoinType m_joinType;
    /* 0x00EC */ u8 _00EC[0x291C - 0x00EC];
    /* 0x291C */ int m_currentConnectionInfoIndex;
    /* 0x2920 */ u8 _2920[0x29C8 - 0x2920];

    static RKNetController* s_instance
        AT(RMCXD_PORT(0x809C20D8, 0x809BD918, 0x809C1138, 0x809B0718));
};

static_assert(sizeof(RKNetController) == 0x29C8);

#endif

} // namespace mkw::Net

#pragma once

#include "import/dwc.h"
#include "import/mkw/hostSystem.hpp"
#include "import/mkw/system/system.hpp"
#include <wwfcGPReport.hpp>

namespace mkw::Net
{

#if RMC

struct __attribute__((packed)) RacePacket {
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
class NetController
{
public:
    enum class JoinType {
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

    void sendRacePacket()
    {
        LONGCALL void sendRacePacket(NetController * netController)
            AT(RMCXD_PORT(0x80657E30, 0x806539A8, 0x8065749C, 0x80646148));

        sendRacePacket(this);
    }

    void
    processRacePacket(u32 playerAid, RacePacket* racePacket, u32 packetSize)
    {
        LONGCALL void processRacePacket(
            NetController * netController, u32 playerAid,
            RacePacket * racePacket, u32 packetSize
        ) AT(RMCXD_PORT(0x80659A84, 0x806555FC, 0x806590F0, 0x80647D9C));

        processRacePacket(this, playerAid, racePacket, packetSize);
    }

    JoinType joinType() const
    {
        return m_joinType;
    }

    u32 availableAids() const
    {
        return currentConnectionInfo().availableAids;
    }

    u8 myAid() const
    {
        return currentConnectionInfo().myAid;
    }

    bool isAidTheServer(u8 playerAid) const
    {
        return playerAid == currentConnectionInfo().serverAid;
    }

    bool amITheServer() const
    {
        return isAidTheServer(myAid());
    }

    bool amITheRoomHost() const
    {
        return m_joinType == JoinType::RoomHost;
    }

    bool usingCustomRegion() const
    {
        using namespace mkw::HostSystem;

        // Allow clients to modify their region without having to change
        // their matching area.
        extern SystemManager::MatchingArea customMatchingArea AT(0x80005EFC);
        if (customMatchingArea < SystemManager::MatchingArea::Japan ||
            customMatchingArea > SystemManager::MatchingArea::China) {
            return true;
        }

        SystemManager::MatchingArea matchingArea =
            SystemManager::Instance()->matchingArea();
        return matchingArea < SystemManager::MatchingArea::Japan ||
               matchingArea > SystemManager::MatchingArea::China;
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
            return !usingCustomRegion();
        }
        case JoinType::RoomHost:
        case JoinType::RoomGuest: {
            return false;
        }
        default:
            return false;
        }
    }

    bool inVanillaRaceScene() const
    {
        using namespace mkw::System;

        int sceneId =
            System::System::Instance().sceneManager()->getCurrentSceneID();
        if (static_cast<Scene::SceneID>(sceneId) != Scene::SceneID::Race) {
            return false;
        }

        return inVanillaMatch();
    }

    void reportAndKick(const char* key, u32 playerAid) const
    {
        using namespace DWC;

        bool alreadyReportedAid = s_reportedAids & (1 << playerAid);
        if (alreadyReportedAid) {
            return;
        }
        s_reportedAids |= (1 << playerAid);

        DWCiNodeInfo* nodeInfo = DWCi_NodeInfoList_GetNodeInfoForAid(playerAid);
        if (nodeInfo) {
            wwfc::GPReport::ReportU32(key, nodeInfo->profileId);
        }

        if (amITheServer()) {
            DWC_CloseConnectionHard(playerAid);
        }
    }

    static void ClearReportedAid(u8 playerAid)
    {
        s_reportedAids &= ~(1 << playerAid);
    }

    static NetController* Instance()
    {
        return s_instance;
    }

private:
    struct ConnectionInfo {
        /* 0x00 */ u8 _00[0x10 - 0x00];
        /* 0x10 */ u32 availableAids;
        /* 0x14 */ u8 _14[0x21 - 0x14];
        /* 0x21 */ u8 myAid;
        /* 0x22 */ u8 serverAid;
        /* 0x23 */ u8 _23[0x58 - 0x23];
    };

    static_assert(sizeof(ConnectionInfo) == 0x58);

    const ConnectionInfo& currentConnectionInfo() const
    {
        return m_connectionInfo[m_currentConnectionInfoIndex];
    }

    /* 0x0000 */ u8 _0000[0x0038 - 0x0000];
    /* 0x0038 */ ConnectionInfo m_connectionInfo[2];
    /* 0x00E8 */ JoinType m_joinType;
    /* 0x00EC */ u8 _00EC[0x291C - 0x00EC];
    /* 0x291C */ int m_currentConnectionInfoIndex;
    /* 0x2920 */ u8 _2920[0x29B0 - 0x2920];

public:
    /* 0x29B0 */ u32 _29B0;
    /* 0x29B4 */ u32 _29B4;
    /* 0x29B8 */ u32 _29B8;
    /* 0x29BC */ u32 _29BC;

private:
    /* 0x29C0 */ u8 _29C0[0x29C8 - 0x29C0];

    static u32 s_reportedAids;

    static NetController* s_instance
        AT(RMCXD_PORT(0x809C20D8, 0x809BD918, 0x809C1138, 0x809B0718));
};

static_assert(sizeof(NetController) == 0x29C8);

#endif

} // namespace mkw::Net

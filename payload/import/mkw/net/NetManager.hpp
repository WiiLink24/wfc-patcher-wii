#pragma once

#if RMC

#  include "import/dwc.h"
#  include "import/mkw/boot/SystemManager.hpp"
#  include "import/mkw/system/System.hpp"
#  include <wwfcGPReport.hpp>
#  include <wwfcPayload.hpp>

namespace wwfc::mkw
{

struct __attribute__((packed)) NetRacePacket {
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
    /* 0x08 */ u8  sizes[8];
};

static_assert(sizeof(NetRacePacket) == 0x10);

// https://github.com/SeekyCt/mkw-structures/blob/master/rknetcontroller.h
class NetManager
{
public:
    enum class EJoinType {
        NOT_JOINING           = 0,
        WW_VS                 = 1,
        CONTINENTAL_VS        = 2,
        WW_BT                 = 3,
        CONTINENTAL_BT        = 4,
        ROOM_HOST             = 5,
        ROOM_GUEST            = 6,
        FRIEND_WW_VS          = 7,
        FRIEND_CONTINENTAL_VS = 8,
        FRIEND_WW_BT          = 9,
        FRIEND_CONTINENTAL_BT = 10,
    };

    void sendRacePacket()
    {
        [[gnu::longcall]] void sendRacePacket(NetManager * netController)
            AT(RMCXD_PORT(0x80657E30, 0x806539A8, 0x8065749C, 0x80646148, 0x80658374));

        sendRacePacket(this);
    }

    void processRacePacket(u32 playerAid, NetRacePacket* racePacket, u32 packetSize)
    {
        [[gnu::longcall]] void processRacePacket(
            NetManager * netController, u32 playerAid, NetRacePacket * racePacket, u32 packetSize
        ) AT(RMCXD_PORT(0x80659A84, 0x806555FC, 0x806590F0, 0x80647D9C, 0x80659FC8));

        processRacePacket(this, playerAid, racePacket, packetSize);
    }

    EJoinType joinType() const
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
        return m_joinType == EJoinType::ROOM_HOST;
    }

    bool usingCustomRegion() const
    {
        // Allow clients to modify their region without having to change
        // their matching area.
        extern SystemManager::EWifiRegion customMatchingArea AT(0x80005EFC);
        if (customMatchingArea < SystemManager::EWifiRegion::JP ||
            customMatchingArea > SystemManager::EWifiRegion::CH) {
            return true;
        }

        SystemManager::EWifiRegion matchingArea = SystemManager::Instance()->getWifiRegion();
        return matchingArea < SystemManager::EWifiRegion::JP ||
               matchingArea > SystemManager::EWifiRegion::CH;
    }

    bool inVanillaMatch() const
    {
        switch (m_joinType) {
        case EJoinType::WW_VS:
        case EJoinType::WW_BT:
        case EJoinType::FRIEND_WW_VS:
        case EJoinType::FRIEND_WW_BT:
            return true;

        case EJoinType::CONTINENTAL_VS:
        case EJoinType::CONTINENTAL_BT:
        case EJoinType::FRIEND_CONTINENTAL_VS:
        case EJoinType::FRIEND_CONTINENTAL_BT:
            return !usingCustomRegion();

        case EJoinType::ROOM_HOST:
        case EJoinType::ROOM_GUEST:
            return false;

        default:
            return false;
        }
    }

    bool isEnableAggressivePacketChecks() const
    {
        if (wwfc::Payload::g_enableAggressivePacketChecks != WWFC_BOOLEAN_RESET) {
            return wwfc::Payload::g_enableAggressivePacketChecks != WWFC_BOOLEAN_FALSE;
        }

        return inVanillaMatch();
    }

    bool inVanillaRaceScene() const
    {
        if (!System::Instance().isCurrentSceneID(Scene::ESceneID::RACE)) {
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

    static NetManager* Instance()
    {
        return s_instance;
    }

private:
    struct ConnectionInfo {
        /* 0x00 */ u8  _00[0x10 - 0x00];
        /* 0x10 */ u32 availableAids;
        /* 0x14 */ u8  _14[0x21 - 0x14];
        /* 0x21 */ u8  myAid;
        /* 0x22 */ u8  serverAid;
        /* 0x23 */ u8  _23[0x58 - 0x23];
    };

    static_assert(sizeof(ConnectionInfo) == 0x58);

    const ConnectionInfo& currentConnectionInfo() const
    {
        return m_connectionInfo[m_currentConnectionInfoIndex];
    }

    /* 0x0000 */ u8             _0000[0x0038 - 0x0000];
    /* 0x0038 */ ConnectionInfo m_connectionInfo[2];
    /* 0x00E8 */ EJoinType      m_joinType;
    /* 0x00EC */ u8             _00EC[0x291C - 0x00EC];
    /* 0x291C */ int            m_currentConnectionInfoIndex;
    /* 0x2920 */ u8             _2920[0x29B0 - 0x2920];

public:
    /* 0x29B0 */ u32 _29B0;
    /* 0x29B4 */ u32 _29B4;
    /* 0x29B8 */ u32 _29B8;
    /* 0x29BC */ u32 _29BC;

private:
    /* 0x29C0 */ u8 _29C0[0x29C8 - 0x29C0];

    static u32 s_reportedAids;

    static NetManager*
        s_instance AT(RMCXD_PORT(0x809C20D8, 0x809BD918, 0x809C1138, 0x809B0718, 0x809C2970));
};

static_assert(sizeof(NetManager) == 0x29C8);

} // namespace wwfc::mkw

#endif // RMC
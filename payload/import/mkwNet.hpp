#pragma once

#include "mkwHostSystem.hpp"
#include "mkwItem.hpp"
#include <wwfcMii.hpp>

namespace mkw::Net
{

#if RMC

struct RACEPacket {
    enum EType {
        HEADER,
        RACEHEADER_1,
        RACEHEADER_2,
        ROOM_SELECT,
        RACEDATA,
        USER,
        ITEM,
        EVENT,
    };

    /* 0x00 */ u32 pad;
    /* 0x04 */ u32 checksum;
    /* 0x08 */ u8 sizes[8];
};

static_assert(sizeof(RACEPacket) == 0x10);

// https://github.com/SeekyCt/mkw-structures/blob/master/roomhandler.h
class ROOMHandler
{
public:
    struct Packet {
        enum class Event : u8 {
            StartRoom = 1,
            RegisterFriend = 2,
            JoinMessage = 3,
            ChatMessage = 4,
        };

        /* 0x00 */ Event event;
        /* 0x01 */ u8 _01[0x04 - 0x01];
    };

    static_assert(sizeof(Packet) == 0x04);

    static ROOMHandler* Instance()
    {
        return s_instance;
    }

private:
    /* 0x00 */ u8 _00[0x80 - 0x00];

    static ROOMHandler* s_instance
        AT(RMCXD_PORT(0x809C20E0, 0x809BD920, 0x809C1140, 0x809B0720));
};

static_assert(sizeof(ROOMHandler) == 0x80);

// https://github.com/SeekyCt/mkw-structures/blob/master/selecthandler.h
class SELECTHandler
{
public:
    struct Packet {
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

        enum class SelectedCourse : u8 {
            NotSelected = 0xFF,
        };

        enum class EngineClass : u8 {
            e100cc = 1,
            e150cc = 2,
            eMirrorMode = 3,
        };

        struct Player {
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
        /* 0x35 */ u8 _35[0x37 - 0x35];
        /* 0x37 */ EngineClass engineClass;
    };

    static_assert(sizeof(Packet) == 0x38);

    void decideEngineClass()
    {
        LONGCALL void decideEngineClass(SELECTHandler * selectHandler)
            AT(RMCXD_PORT(0x80661A5C, 0x80659B20, 0x806610C8, 0x8064FD74));

        decideEngineClass(this);
    }

    Packet& sendPacket()
    {
        return m_sendPacket;
    }

    static SELECTHandler* Instance()
    {
        return s_instance;
    }

private:
    /* 0x000 */ u8 _000[0x008 - 0x000];
    /* 0x008 */ Packet m_sendPacket;
    /* 0x040 */ u8 _040[0x3F8 - 0x040];

    static SELECTHandler* s_instance
        AT(RMCXD_PORT(0x809C2100, 0x809BD930, 0x809C1160, 0x809B0740));
};

static_assert(sizeof(SELECTHandler) == 0x3F8);

struct USERPacket {
    /* 0x00 */ u32 miiGroupBitflags;
    /* 0x04 */ u16 miiGroupCount;
    /* 0x06 */ u16 _0x06;
    /* 0x08 */ wwfc::Mii::RFLiStoreData miiData[2];
    /* 0xA0 */ u64 wiiFriendCode;
    /* 0xA8 */ u64 friendCode;
    /* 0xB0 */ u8 country;
    /* 0xB1 */ u8 state;
    /* 0xB2 */ u16 city;
    /* 0xB4 */ u16 longitude;
    /* 0xB6 */ u16 latitude;
    /* 0xB8 */ u16 vr;
    /* 0xBA */ u16 br;
    /* 0xBC */ char regionChar;
    /* 0xBD */ u8 regionId;
    /* 0xBE */ u16 _0xBE;
};

static_assert(sizeof(USERPacket) == 0xC0);

// https://github.com/SeekyCt/mkw-structures/blob/master/rknetcontroller.h
class ITEMHandler
{
public:
    struct Packet {
        enum class Item : u8 {
            GreenShell = 0x00,
            RedShell = 0x01,
            Banana = 0x02,
            FakeItemBox = 0x03,
            Mushroom = 0x04,
            TripleMushrooms = 0x05,
            Bob_omb = 0x06,
            BlueShell = 0x07,
            Lightning = 0x08,
            Star = 0x09,
            GoldenMushroom = 0x0A,
            MegaMushroom = 0x0B,
            Blooper = 0x0C,
            POWBlock = 0x0D,
            ThunderCloud = 0x0E,
            BulletBill = 0x0F,
            TripleGreenShells = 0x10,
            TripleRedShells = 0x11,
            TripleBananas = 0x12,
            NoItem = 0x14,
        };

        constexpr bool isHeldItemValidVS() const
        {
            return isItemValidVS(heldItem);
        }

        constexpr bool isTrailedItemValidVS() const
        {
            if (trailedItem == Item::NoItem) {
                return true;
            }

            if (!isItemValidVS(trailedItem)) {
                return false;
            }

            return canTrailItem(trailedItem);
        }

        constexpr bool isHeldItemValidBB() const
        {
            return isItemValidBB(heldItem);
        }

        constexpr bool isTrailedItemValidBB() const
        {
            if (trailedItem == Item::NoItem) {
                return true;
            }

            if (!isItemValidBB(trailedItem)) {
                return false;
            }

            return canTrailItem(trailedItem);
        }

        constexpr bool isHeldItemValidCR() const
        {
            return isItemValidCR(heldItem);
        }

        constexpr bool isTrailedItemValidCR() const
        {
            if (trailedItem == Item::NoItem) {
                return true;
            }

            if (!isItemValidCR(trailedItem)) {
                return false;
            }

            return canTrailItem(trailedItem);
        }

        /* 0x00 */ u8 _00;
        /* 0x01 */ Item heldItem;
        /* 0x02 */ Item trailedItem;
        /* 0x03 */ u8 _03[0x08 - 0x03];

    private:
        constexpr bool isItemValidVS(Item item) const
        {
            switch (item) {
            case Item::GreenShell... Item::TripleBananas:
            case Item::NoItem: {
                return true;
            }
            default: {
                return false;
            }
            }
        }

        constexpr bool isItemValidBB(Item item) const
        {
            switch (item) {
            case Item::GreenShell... Item::Star:
            case Item::MegaMushroom... Item::Blooper:
            case Item::TripleGreenShells... Item::TripleBananas:
            case Item::NoItem: {
                return true;
            }
            default: {
                return false;
            }
            }
        }

        constexpr bool isItemValidCR(Item item) const
        {
            switch (item) {
            case Item::GreenShell... Item::BlueShell:
            case Item::Star... Item::POWBlock:
            case Item::TripleGreenShells... Item::TripleBananas:
            case Item::NoItem: {
                return true;
            }
            default: {
                return false;
            }
            }
        }

        constexpr bool canTrailItem(Item item) const
        {
            u8 trailedItem = static_cast<u8>(item);

            const mkw::Item::ItemBehaviourEntry& itemBehaviourEntry =
                mkw::Item::itemBehaviourTable[trailedItem];

            return !itemBehaviourEntry.useFunction;
        }
    };

    static_assert(sizeof(Packet) == 0x08);

    static ITEMHandler* Instance()
    {
        return s_instance;
    }

private:
    u8 _000[0x184 - 0x000];

    static ITEMHandler* s_instance
        AT(RMCXD_PORT(0x809C20F8, 0x809BD950, 0x809C1158, 0x809B0738));
};

static_assert(sizeof(ITEMHandler) == 0x184);

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
        /* 0x00 */ u8 _00[0x22 - 0x00];
        /* 0x22 */ u8 hostAid;
        /* 0x23 */ u8 _23[0x58 - 0x23];
    };

    static_assert(sizeof(ConnectionInfo) == 0x58);

    void
    processRACEPacket(u32 playerAid, RACEPacket* racePacket, u32 packetSize)
    {
        LONGCALL void processRACEPacket(
            RKNetController * rkNetController, u32 playerAid,
            RACEPacket * racePacket, u32 packetSize
        ) AT(RMCXD_PORT(0x80659A84, 0x806555FC, 0x806590F0, 0x80647D9C));

        processRACEPacket(this, playerAid, racePacket, packetSize);
    }

    ConnectionInfo& currentConnectionInfo()
    {
        return m_connectionInfo[m_currentConnectionInfoIndex];
    }

    constexpr JoinType joinType() const
    {
        return m_joinType;
    }

    constexpr bool inVanillaMatch() const
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
            auto matchingArea =
                mkw::HostSystem::SystemManager::Instance()->matchingArea();

            return matchingArea >=
                       mkw::HostSystem::SystemManager::MatchingArea::Japan &&
                   matchingArea <=
                       mkw::HostSystem::SystemManager::MatchingArea::China;
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

#include "import/mkw/registry.hpp"
#if RMC

#  include "import/mkw/Registry.hpp"
#  include "import/mkw/net/NetEventHandler.hpp"
#  include "import/mkw/net/NetItemHandler.hpp"
#  include "import/mkw/net/NetManager.hpp"
#  include "import/mkw/net/NetMatchHeaderHandler.hpp"
#  include "import/mkw/net/NetRoomHandler.hpp"
#  include "import/mkw/net/NetSelectHandler.hpp"
#  include "import/mkw/net/NetUserHandler.hpp"
#  include "import/mkw/system/RaceConfig.hpp"
#  include "import/mkw/system/System.hpp"
#  include "wwfcLibC.hpp"
#  include "wwfcLog.hpp"
#  include "wwfcPayload.hpp"
#  include "wwfcTypes.h"

namespace wwfc::Security
{

using namespace mkw;

static std::size_t s_vanillaPacketBufferSizes[sizeof(NetRacePacket::sizes)] = {
    sizeof(NetRacePacket),
    sizeof(NetMatchHeaderHandler::Packet),
    0x28,
    sizeof(NetSelectHandler::Packet),
    0x40 << 1,
    sizeof(NetUserHandler::Packet),
    sizeof(NetItemHandler::Packet) << 1,
    sizeof(NetEventHandler::Packet),
};

static bool IsPacketSizeValid(NetRacePacket::EType packetType, u8 packetSize)
{
    if (packetType >= NetRacePacket::MatchHeader && packetType <= NetRacePacket::Event) {
        if (packetSize == 0) {
            return true;
        }
    }

    std::size_t* packetBufferSizesPointer;
    if (!NetManager::Instance()->isEnableAggressivePacketChecks()) {
        extern std::size_t packetBufferSizes[sizeof(NetRacePacket::sizes)] AT(
            RMCXD_PORT(0x8089A194, 0x80895AC4, 0x808992F4, 0x808885CC, 0x8089A864)
        );

        packetBufferSizesPointer = packetBufferSizes;
    } else {
        packetBufferSizesPointer = s_vanillaPacketBufferSizes;
    }

    switch (packetType) {
    case NetRacePacket::Header: {
        if (packetSize < sizeof(NetRacePacket) ||
            packetSize > packetBufferSizesPointer[NetRacePacket::Header]) {
            return false;
        }

        return true;
    }
    case NetRacePacket::MatchHeader: {
        if (packetSize < sizeof(NetMatchHeaderHandler::Packet) ||
            packetSize > packetBufferSizesPointer[NetRacePacket::MatchHeader]) {
            return false;
        }

        return true;
    }
    case NetRacePacket::MatchData: {
        if (packetSize < 0x28 || packetSize > packetBufferSizesPointer[NetRacePacket::MatchData]) {
            return false;
        }

        return true;
    }
    case NetRacePacket::RoomSelect: {
        // 'Room' packet
        if (packetSize < sizeof(NetSelectHandler::Packet)) {
            return packetSize >= sizeof(NetRoomHandler::Packet);
        }

        // 'Select' packet
        if (packetSize > packetBufferSizesPointer[NetRacePacket::RoomSelect]) {
            return false;
        }

        return true;
    }
    case NetRacePacket::PlayerData: {
        if (packetSize < 0x40 || packetSize > packetBufferSizesPointer[NetRacePacket::PlayerData]) {
            return false;
        }

        return true;
    }
    case NetRacePacket::User: {
        if (packetSize < sizeof(NetUserHandler::Packet) ||
            packetSize > packetBufferSizesPointer[NetRacePacket::User]) {
            return false;
        }

        return true;
    }
    case NetRacePacket::Item: {
        if (packetSize < sizeof(NetItemHandler::Packet) ||
            packetSize > packetBufferSizesPointer[NetRacePacket::Item]) {
            return false;
        }

        return true;
    }
    case NetRacePacket::Event: {
        if (packetSize < sizeof(NetEventHandler::Packet::eventInfo) ||
            packetSize > packetBufferSizesPointer[NetRacePacket::Event]) {
            return false;
        }

        return true;
    }
    default: {
        return false;
    }
    }
}

static bool IsHeaderPacketDataValid(
    const void* /* packet */, u8 /* packetSize */, u8 /* playerAid */
)
{
    // This packet is validated by the function 'IsRacePacketValid'
    return true;
}

static bool IsMatchHeaderPacketDataValid(
    const void* packet, u8 /* packetSize */, u8 /* playerAid */
)
{
    const NetMatchHeaderHandler::Packet* matchHeaderPacket =
        static_cast<const NetMatchHeaderHandler::Packet*>(packet);

    if (wwfc::Payload::g_enableAggressivePacketChecks == WWFC_BOOLEAN_FALSE ||
        (wwfc::Payload::g_enableAggressivePacketChecks == WWFC_BOOLEAN_RESET &&
         !NetManager::Instance()->inVanillaRaceScene())) {
        return true;
    }

    RaceConfig* config = &RaceConfigManager::Instance()->getConfig();

    for (std::size_t n = 0; n < std::size(matchHeaderPacket->player); n++) {
        NetMatchHeaderHandler::Packet::Player player = matchHeaderPacket->player[n];

        if (static_cast<u8>(player.vehicle) == NetMatchHeaderHandler::Packet::NONE &&
            static_cast<u8>(player.character) == NetMatchHeaderHandler::Packet::NONE) {
            continue;
        }

        if (config->isWifiVSRace()) {
            if (!IsCombinationValidVS(player.character, player.vehicle)) {
                return false;
            }
        } else /* if (config->isOnlineBattle()) */ {
            if (!IsCombinationValidBT(player.character, player.vehicle)) {
                return false;
            }
        }
    }

    if (static_cast<u8>(matchHeaderPacket->course) == NetMatchHeaderHandler::Packet::NONE) {
        return true;
    }

    if (config->isWifiVSRace()) {
        return IsRaceCourse(matchHeaderPacket->course);
    } else /* if (config->isOnlineBattle()) */ {
        return IsBattleCourse(matchHeaderPacket->course);
    }
}

static bool IsMatchDataPacketDataValid(
    const void* /* packet */, u8 /* packetSize */, u8 /* playerAid */
)
{
    return true;
}

static bool IsRoomSelectPacketDataValid(const void* packet, u8 packetSize, u8 playerAid)
{
    // 'Room' packet
    if (packetSize == sizeof(NetRoomHandler::Packet)) {
        const NetRoomHandler::Packet* roomPacket =
            static_cast<const NetRoomHandler::Packet*>(packet);

        switch (roomPacket->event) {
        case NetRoomHandler::Packet::EEvent::START_ROOM:
            // Ensure that guests can't start rooms
            if (!NetManager::Instance()->isAidTheServer(playerAid)) {
                return false;
            }
            break;

        default:
            break;
        }
    }
    // 'Select' packet

    if (!NetManager::Instance()->isEnableAggressivePacketChecks()) {
        return true;
    }

    RaceConfig* config = System::Instance().isCurrentSceneID(Scene::ESceneID::RACE)
                             ? &RaceConfigManager::Instance()->getConfig()
                             : &RaceConfigManager::Instance()->getConfigNext();

    const NetSelectHandler::Packet* selectPacket =
        static_cast<const NetSelectHandler::Packet*>(packet);
    for (std::size_t n = 0; n < std::size(selectPacket->player); n++) {
        NetSelectHandler::Packet::Player player = selectPacket->player[n];

        if (player.character != ECharacter::CONTROL_VOTING_NOT_SELECTED ||
            player.vehicle != EVehicle::CONTROL_VOTING_NOT_SELECTED) {
            if (config->isWifiVSRace()) {
                if (!IsCombinationValidVS(player.character, player.vehicle)) {
                    return false;
                }
            } else /* if (config->isOnlineBattle()) */ {
                if (!IsCombinationValidBT(player.character, player.vehicle)) {
                    return false;
                }
            }
        }

        ECourse course = selectPacket->player[n].courseVote;
        if (course == ECourse::CONTROL_VOTING_NOT_SELECTED ||
            course == ECourse::CONTROL_VOTING_RANDOM) {
            continue;
        }
        if (config->isWifiVSRace()) {
            if (!IsRaceCourse(course)) {
                return false;
            }
        } else /* if (config->isOnlineBattle()) */ {
            if (!IsBattleCourse(course)) {
                return false;
            }
        }
    }

    if (selectPacket->selectedCourse == ECourse::CONTROL_VOTING_NOT_DECIDED) {
        return true;
    }

    if (config->isWifiVSRace()) {
        return IsRaceCourse(selectPacket->selectedCourse);
    } else /* if (config->isOnlineBattle()) */ {
        return IsBattleCourse(selectPacket->selectedCourse);
    }
}

static bool IsPlayerDataPacketDataValid(
    const void* /* packet */, u8 /* packetSize */, u8 /* playerAid */
)
{
    return true;
}

static bool IsUserPacketDataValid(
    const void* packet, u8 /* packetSize */, u8 /* playerAid */
)
{
    const NetUserHandler::Packet* userPacket = static_cast<const NetUserHandler::Packet*>(packet);

    return userPacket->isValid();
}

static bool IsItemPacketDataValid(const void* packet, u8 packetSize, u8 /* playerAid */)
{
    if (wwfc::Payload::g_enableAggressivePacketChecks == WWFC_BOOLEAN_FALSE ||
        (wwfc::Payload::g_enableAggressivePacketChecks == WWFC_BOOLEAN_RESET &&
         !NetManager::Instance()->inVanillaRaceScene())) {
        return true;
    }

    RaceConfig* config = &RaceConfigManager::Instance()->getConfig();

    for (u8 n = 0; n < packetSize / sizeof(NetItemHandler::Packet); n++) {
        const NetItemHandler::Packet* const itemPacket =
            static_cast<const NetItemHandler::Packet*>(packet) + n;

        const EItemType heldItem    = itemPacket->heldItem;
        const EItemType trailedItem = itemPacket->trailedItem;

        if (config->isWifiVSRace()) {
            if (heldItem != EItemType::EMPTY && !ItemDefaults::isValidVS(heldItem)) {
                return false;
            }
            if (trailedItem != EItemType::EMPTY && !ItemDefaults::isTrailValidVS(trailedItem)) {
                return false;
            }
        } else /* if (config->isOnlineBattle()) */ {
            if (config->isBattleBalloon()) {
                if (heldItem != EItemType::EMPTY && !ItemDefaults::isValidBTBalloon(heldItem)) {
                    return false;
                }
                if (trailedItem != EItemType::EMPTY &&
                    !ItemDefaults::isTrailValidBTBalloon(trailedItem)) {
                    return false;
                }
            } else /* if (config->isCoinRunners()) */ {
                if (heldItem != EItemType::EMPTY && !ItemDefaults::isValidBTCoin(heldItem)) {
                    return false;
                }
                if (trailedItem != EItemType::EMPTY &&
                    !ItemDefaults::isTrailValidBTCoin(trailedItem)) {
                    return false;
                }
            }
        }

        if (!itemPacket->isHeldPhaseValid() || !itemPacket->isTrailPhaseValid()) {
            return false;
        }
    }

    return true;
}

static bool IsEventPacketDataValid(const void* packet, u8 packetSize, u8 playerAid)
{
    if (!System::Instance().isCurrentSceneID(Scene::ESceneID::RACE)) {
        return true;
    }

    const NetEventHandler::Packet* eventPacket =
        static_cast<const NetEventHandler::Packet*>(packet);
    const bool packetChecks = NetManager::Instance()->isEnableAggressivePacketChecks();

    // Always ensure that the packet does not contain any invalid item
    // objects, as this can cause a buffer overflow to occur.
    if (wwfc::Payload::g_enableEventItemIdCheck || packetChecks) {
        if (eventPacket->containsInvalidItemObject()) {
            return false;
        }
    }

    return !packetChecks || eventPacket->isValid(packetSize, playerAid);
}

typedef bool (*IsPacketDataValid)(const void* packet, u8 packetSize, u8 playerAid);

static std::array<IsPacketDataValid, sizeof(NetRacePacket::sizes)> s_isPacketDataValid{
    IsHeaderPacketDataValid,     IsMatchHeaderPacketDataValid, IsMatchDataPacketDataValid,
    IsRoomSelectPacketDataValid, IsPlayerDataPacketDataValid,  IsUserPacketDataValid,
    IsItemPacketDataValid,       IsEventPacketDataValid,
};

bool IsRacePacketValid(const NetRacePacket* racePacket, u32 racePacketSize, u8 playerAid)
{
    if (racePacketSize < sizeof(NetRacePacket)) {
        return false;
    }

    u32 expectedPacketSize = 0;
    for (std::size_t n = 0; n < std::size(racePacket->sizes); n++) {
        NetRacePacket::EType packetType = static_cast<NetRacePacket::EType>(n);
        u8                   packetSize = racePacket->sizes[n];

        if (!IsPacketSizeValid(packetType, packetSize)) {
            WWFC_LOG_WARN_FMT("Invalid packet size (type %d size 0x%X)", n, packetSize);
            return false;
        }

        expectedPacketSize += packetSize;
    }

    if (racePacketSize < expectedPacketSize) {
        WWFC_LOG_WARN_FMT(
            "Invalid packet size (expected 0x%X got 0x%X)", expectedPacketSize, racePacketSize
        );
        return false;
    }

    expectedPacketSize = 0;
    for (std::size_t n = 0; n < std::size(racePacket->sizes); n++) {
        const IsPacketDataValid isPacketDataValid = s_isPacketDataValid[n];
        const void* packet     = reinterpret_cast<const char*>(racePacket) + expectedPacketSize;
        u8          packetSize = racePacket->sizes[n];

        if (packetSize != 0 && !isPacketDataValid(packet, packetSize, playerAid)) {
            WWFC_LOG_WARN_FMT("Invalid packet data (type %d)", n);
            return false;
        }

        expectedPacketSize += packetSize;
    }

    return true;
}

} // namespace wwfc::Security

#endif // RMC
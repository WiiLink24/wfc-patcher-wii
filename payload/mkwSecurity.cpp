#if RMC

#  include "import/mkw/net/eventHandler.hpp"
#  include "import/mkw/net/itemHandler.hpp"
#  include "import/mkw/net/matchHeaderHandler.hpp"
#  include "import/mkw/net/net.hpp"
#  include "import/mkw/net/roomHandler.hpp"
#  include "import/mkw/net/selectHandler.hpp"
#  include "import/mkw/net/userHandler.hpp"
#  include "import/mkw/registry.hpp"
#  include "import/mkw/system/raceConfig.hpp"
#  include "import/mkw/system/system.hpp"
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
    if (packetType >= NetRacePacket::MatchHeader &&
        packetType <= NetRacePacket::Event) {
        if (packetSize == 0) {
            return true;
        }
    }

    std::size_t* packetBufferSizesPointer;
    if (!NetController::Instance()->isEnableAggressivePacketChecks()) {
        extern std::size_t packetBufferSizes[sizeof(NetRacePacket::sizes)] AT(
            RMCXD_PORT(
                0x8089A194, 0x80895AC4, 0x808992F4, 0x808885CC, 0x8089A864
            )
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
        if (packetSize < 0x28 ||
            packetSize > packetBufferSizesPointer[NetRacePacket::MatchData]) {
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
        if (packetSize < 0x40 ||
            packetSize > packetBufferSizesPointer[NetRacePacket::PlayerData]) {
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
    using namespace mkw::Registry;

    const NetMatchHeaderHandler::Packet* matchHeaderPacket =
        reinterpret_cast<const NetMatchHeaderHandler::Packet*>(packet);

    if (wwfc::Payload::g_enableAggressivePacketChecks == WWFC_BOOLEAN_FALSE ||
        (wwfc::Payload::g_enableAggressivePacketChecks == WWFC_BOOLEAN_RESET &&
         !NetController::Instance()->inVanillaRaceScene())) {
        return true;
    }

    RaceConfig::Scenario* scenario = &RaceConfig::Instance()->raceScenario();

    for (std::size_t n = 0; n < std::size(matchHeaderPacket->player); n++) {
        NetMatchHeaderHandler::Packet::Player player =
            matchHeaderPacket->player[n];

        NetMatchHeaderHandler::Packet::Vehicle playerVehicle = player.vehicle;
        NetMatchHeaderHandler::Packet::Character playerCharacter =
            player.character;
        if (playerVehicle == NetMatchHeaderHandler::Packet::Vehicle::None &&
            playerCharacter == NetMatchHeaderHandler::Packet::Character::None) {
            continue;
        }

        Vehicle vehicle = static_cast<Vehicle>(playerVehicle);
        Character character = static_cast<Character>(playerCharacter);
        if (scenario->isOnlineVersusRace()) {
            if (!IsCombinationValidVS(character, vehicle)) {
                return false;
            }
        } else /* if (scenario->isOnlineBattle()) */ {
            if (!IsCombinationValidBT(character, vehicle)) {
                return false;
            }
        }
    }

    NetMatchHeaderHandler::Packet::Course currentCourse =
        matchHeaderPacket->course;
    if (currentCourse != NetMatchHeaderHandler::Packet::Course::None) {
        Course course = static_cast<Course>(currentCourse);
        if (scenario->isOnlineVersusRace()) {
            if (!IsRaceCourse(course)) {
                return false;
            }
        } else /* if (scenario->isOnlineBattle()) */ {
            if (!IsBattleCourse(course)) {
                return false;
            }
        }
    }

    return true;
}

static bool IsMatchDataPacketDataValid(
    const void* /* packet */, u8 /* packetSize */, u8 /* playerAid */
)
{
    return true;
}

static bool
IsRoomSelectPacketDataValid(const void* packet, u8 packetSize, u8 playerAid)
{
    // 'Room' packet
    if (packetSize == sizeof(NetRoomHandler::Packet)) {
        const NetRoomHandler::Packet* roomPacket =
            reinterpret_cast<const NetRoomHandler::Packet*>(packet);

        switch (roomPacket->event) {
        case NetRoomHandler::Packet::Event::StartRoom: {
            // Ensure that guests can't start rooms
            if (!NetController::Instance()->isAidTheServer(playerAid)) {
                return false;
            }
            break;
        }
        default: {
            break;
        }
        }
    }
    // 'Select' packet
    else {
        if (!NetController::Instance()->isEnableAggressivePacketChecks()) {
            return true;
        }

        RaceConfig::Scenario* scenario;
        if (static_cast<Scene::SceneID>(
                System::Instance().sceneManager()->getCurrentSceneID()
            ) == Scene::SceneID::Race) {
            scenario = &RaceConfig::Instance()->raceScenario();
        } else {
            scenario = &RaceConfig::Instance()->menuScenario();
        }

        const NetSelectHandler::Packet* selectPacket =
            reinterpret_cast<const NetSelectHandler::Packet*>(packet);
        for (std::size_t n = 0; n < std::size(selectPacket->player); n++) {
            NetSelectHandler::Packet::Player player = selectPacket->player[n];

            NetSelectHandler::Packet::Player::Character selectedCharacter =
                player.character;
            NetSelectHandler::Packet::Player::Vehicle selectedVehicle =
                player.vehicle;
            if (selectedCharacter !=
                    NetSelectHandler::Packet::Player::Character::NotSelected ||
                selectedVehicle !=
                    NetSelectHandler::Packet::Player::Vehicle::NotSelected) {
                Registry::Character character =
                    static_cast<Registry::Character>(selectedCharacter);
                Registry::Vehicle vehicle =
                    static_cast<Registry::Vehicle>(selectedVehicle);
                if (scenario->isOnlineVersusRace()) {
                    if (!IsCombinationValidVS(character, vehicle)) {
                        return false;
                    }
                } else /* if (scenario->isOnlineBattle()) */ {
                    if (!IsCombinationValidBT(character, vehicle)) {
                        return false;
                    }
                }
            }

            NetSelectHandler::Packet::Player::CourseVote courseVote =
                selectPacket->player[n].courseVote;
            if (courseVote ==
                    NetSelectHandler::Packet::Player::CourseVote::NotSelected ||
                courseVote ==
                    NetSelectHandler::Packet::Player::CourseVote::Random) {
                continue;
            }
            Registry::Course course = static_cast<Registry::Course>(courseVote);
            if (scenario->isOnlineVersusRace()) {
                if (!IsRaceCourse(course)) {
                    return false;
                }
            } else /* if (scenario->isOnlineBattle()) */ {
                if (!IsBattleCourse(course)) {
                    return false;
                }
            }
        }

        if (selectPacket->selectedCourse !=
            NetSelectHandler::Packet::SelectedCourse::NotSelected) {
            Registry::Course course =
                static_cast<Registry::Course>(selectPacket->selectedCourse);
            if (scenario->isOnlineVersusRace()) {
                if (!IsRaceCourse(course)) {
                    return false;
                }
            } else /* if (scenario->isOnlineBattle()) */ {
                if (!IsBattleCourse(course)) {
                    return false;
                }
            }
        }
    }

    return true;
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
    const NetUserHandler::Packet* userPacket =
        reinterpret_cast<const NetUserHandler::Packet*>(packet);

    if (!userPacket->isValid()) {
        return false;
    }

    return true;
}

static bool
IsItemPacketDataValid(const void* packet, u8 packetSize, u8 /* playerAid */)
{
    if (wwfc::Payload::g_enableAggressivePacketChecks == WWFC_BOOLEAN_FALSE ||
        (wwfc::Payload::g_enableAggressivePacketChecks == WWFC_BOOLEAN_RESET &&
         !NetController::Instance()->inVanillaRaceScene())) {
        return true;
    }

    RaceConfig::Scenario* scenario = &RaceConfig::Instance()->raceScenario();

    for (u8 n = 0; n < (packetSize >> 3); n++) {
        const NetItemHandler::Packet* itemPacket =
            reinterpret_cast<const NetItemHandler::Packet*>(
                reinterpret_cast<const char*>(packet) +
                (sizeof(NetItemHandler::Packet) * n)
            );

        ItemBox heldItem = static_cast<ItemBox>(itemPacket->heldItem);
        ItemBox trailedItem = static_cast<ItemBox>(itemPacket->trailedItem);

        if (scenario->isOnlineVersusRace()) {
            if (heldItem != ItemBox::NoItem && !IsHeldItemValidVS(heldItem)) {
                return false;
            }
            if (trailedItem != ItemBox::NoItem &&
                !IsTrailedItemValidVS(trailedItem)) {
                return false;
            }
        } else /* if (scenario->isOnlineBattle()) */ {
            if (scenario->isBalloonBattle()) {
                if (heldItem != ItemBox::NoItem &&
                    !IsHeldItemValidBB(heldItem)) {
                    return false;
                }
                if (trailedItem != ItemBox::NoItem &&
                    !IsTrailedItemValidBB(trailedItem)) {
                    return false;
                }
            } else /* if (scenario->isCoinRunners()) */ {
                if (heldItem != ItemBox::NoItem &&
                    !IsHeldItemValidCR(heldItem)) {
                    return false;
                }
                if (trailedItem != ItemBox::NoItem &&
                    !IsTrailedItemValidCR(trailedItem)) {
                    return false;
                }
            }
        }

        if (!itemPacket->isHeldPhaseValid()) {
            return false;
        }
        if (!itemPacket->isTrailPhaseValid()) {
            return false;
        }
    }

    return true;
}

static bool
IsEventPacketDataValid(const void* packet, u8 packetSize, u8 playerAid)
{
    if (static_cast<Scene::SceneID>(
            System::Instance().sceneManager()->getCurrentSceneID()
        ) != Scene::SceneID::Race) {
        return true;
    }

    const NetEventHandler::Packet* eventPacket =
        reinterpret_cast<const NetEventHandler::Packet*>(packet);
    const bool packetChecks =
        NetController::Instance()->isEnableAggressivePacketChecks();

    // Always ensure that the packet does not contain any invalid item
    // objects, as this can cause a buffer overflow to occur.
    if (wwfc::Payload::g_enableEventItemIdCheck || packetChecks) {
        if (eventPacket->containsInvalidItemObject()) {
            return false;
        }
    }

    if (!packetChecks) {
        return true;
    }

    if (!eventPacket->isValid(packetSize, playerAid)) {
        return false;
    }

    return true;
}

typedef bool (*IsPacketDataValid)(
    const void* packet, u8 packetSize, u8 playerAid
);

static std::array<IsPacketDataValid, sizeof(NetRacePacket::sizes)>
    s_isPacketDataValid{
        IsHeaderPacketDataValid,     IsMatchHeaderPacketDataValid,
        IsMatchDataPacketDataValid,  IsRoomSelectPacketDataValid,
        IsPlayerDataPacketDataValid, IsUserPacketDataValid,
        IsItemPacketDataValid,       IsEventPacketDataValid,
    };

bool IsRacePacketValid(
    const NetRacePacket* racePacket, u32 racePacketSize, u8 playerAid
)
{
    if (racePacketSize < sizeof(NetRacePacket)) {
        return false;
    }

    u32 expectedPacketSize = 0;
    for (std::size_t n = 0; n < std::size(racePacket->sizes); n++) {
        NetRacePacket::EType packetType = static_cast<NetRacePacket::EType>(n);
        u8 packetSize = racePacket->sizes[n];

        if (!IsPacketSizeValid(packetType, packetSize)) {
            WWFC_LOG_WARN_FMT(
                "Invalid packet size (type %d size 0x%X)", n, packetSize
            );
            return false;
        }

        expectedPacketSize += packetSize;
    }

    if (racePacketSize < expectedPacketSize) {
        WWFC_LOG_WARN_FMT(
            "Invalid packet size (expected 0x%X got 0x%X)", expectedPacketSize,
            racePacketSize
        );
        return false;
    }

    expectedPacketSize = 0;
    for (std::size_t n = 0; n < std::size(racePacket->sizes); n++) {
        const IsPacketDataValid isPacketDataValid = s_isPacketDataValid[n];
        const void* packet =
            reinterpret_cast<const char*>(racePacket) + expectedPacketSize;
        u8 packetSize = racePacket->sizes[n];

        if (packetSize != 0 &&
            !isPacketDataValid(packet, packetSize, playerAid)) {
            WWFC_LOG_WARN_FMT("Invalid packet data (type %d)", n);
            return false;
        }

        expectedPacketSize += packetSize;
    }

    return true;
}

} // namespace wwfc::Security

#endif // RMC
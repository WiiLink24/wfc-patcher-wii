#include "import/mkw/net/itemHandler.hpp"
#include "import/mkw/net/selectHandler.hpp"
#include "import/mkw/net/userHandler.hpp"
#include "import/mkw/system/gameScene.hpp"
#include "import/mkw/ui/page/friendRoomPage.hpp"
#include "import/mkw/ui/page/wifiFriendMenuPage.hpp"
#include "import/mkw/ui/page/wifiMenuPage.hpp"
#include "wwfcPatch.hpp"
#include <cstring>

namespace mkw::Net
{

#if RMC

u16 RacePacketHandler::s_localLagFrameCount = 0;
u32 SelectHandler::s_kickTimerFrames = 0;

#endif

} // namespace mkw::Net

namespace mkw::UI
{

#if RMC

OpenHostPage::State OpenHostPage::s_state = OpenHostPage::State::Previous;
MenuInputManager::Handler<OpenHostPage>* OpenHostPage::s_onOption = nullptr;
YesNoPage::Handler<OpenHostPage>* OpenHostPage::s_onYesOrNo = nullptr;
bool OpenHostPage::s_openHostEnabled = false;
bool OpenHostPage::s_sentOpenHostValue = false;

const wchar_t* WifiMenuPage::s_message = nullptr;
wchar_t WifiMenuPage::s_messageOfTheDayBuffer[256] = {};
bool WifiMenuPage::s_hasSeenMessageOfTheDay = false;

#endif

} // namespace mkw::UI

namespace wwfc::Feature
{

#if RMC

static const char s_loginChallenge2Message[] = "\\lc\\2";

extern "C" {
__attribute__((__used__)) static GameSpy::GPResult
GetMessageOfTheDay(GameSpy::GPResult gpResult, const char* message)
{
    using namespace mkw::UI;

    if (WifiMenuPage::HasSeenMessageOfTheDay()) {
        return gpResult;
    }

    if (strncmp(
            message, s_loginChallenge2Message,
            sizeof(s_loginChallenge2Message) - 1
        )) {
        return gpResult;
    }

    char value[512];
    if (!GameSpy::gpiValueForKey(
            message, "\\wwfc_motd\\", value, sizeof(value)
        )) {
        return gpResult;
    }

    wchar_t* messageOfTheDayBuffer = WifiMenuPage::MessageOfTheDayBuffer();
    s32 messageOfTheDayBufferSize =
        static_cast<s32>(WifiMenuPage::MessageOfTheDayBufferSize());
    s32 messageOfTheDayLength = DWC::DWC_Base64Decode(
        value, strlen(value), reinterpret_cast<char*>(messageOfTheDayBuffer),
        messageOfTheDayBufferSize
    );
    if (messageOfTheDayLength == -1 ||
        messageOfTheDayLength == messageOfTheDayBufferSize) {
        return gpResult;
    }
    messageOfTheDayBuffer[messageOfTheDayLength / sizeof(wchar_t)] = L'\0';

    WifiMenuPage::SetMessage(messageOfTheDayBuffer);
    WifiMenuPage::SeenMessageOfTheDay();

    return gpResult;
}
}

// Get the Message Of The Day from the "Login Challenge 2" message
WWFC_DEFINE_PATCH = {
    Patch::BranchWithCTR( //
        WWFC_PATCH_LEVEL_FEATURE, //
        RMCXD_PORT(0x80101074, 0x80100FD4, 0x80100F94, 0x801010EC), //
        ASM_LAMBDA(
            // clang-format off
            mr        r4, r26;

            bl        _restgpr_26;
            lwz       r0, 0x2D4(r1);
            mtlr      r0;
            addi      r1, r1, 0x2D0;
            
            b         GetMessageOfTheDay;
            // clang-format on
        )
    ),
};

// Display the message if there is one to display
WWFC_DEFINE_PATCH = {
    Patch::BranchWithCTR( //
        WWFC_PATCH_LEVEL_FEATURE, //
        RMCXD_PORT(0x8064BCD4, 0x806189C0, 0x8064B340, 0x80639FEC), //
        ASM_LAMBDA(
            // clang-format off
            mr        r3, r31;
            
            lwz       r31, 0x0C(r1);
            lwz       r0, 0x14(r1);
            mtlr      r0;
            addi      r1, r1, 0x10;
            
            b         WifiMenuPage_showMessage;
            // clang-format on
        )
    ),
};

// Allow users to open rooms without having any friends added
WWFC_DEFINE_PATCH = {
    Patch::CallWithCTR( //
        WWFC_PATCH_LEVEL_FEATURE, //
        RMCXD_PORT(0x8064D358, 0x8061A044, 0x8064C9C4, 0x8063B670), //
        // clang-format off
        [](mkw::UI::WifiFriendMenuPage* /* wifiFriendMenuPage */, void* /* pushButton */) -> int {
            constexpr int friendsAdded = 1;

            return friendsAdded;
        }
        // clang-format on
    ),
};

// Reset the counter that is used to detect if this client is lagging the room
WWFC_DEFINE_PATCH = {
    Patch::CallWithCTR( //
        WWFC_PATCH_LEVEL_FEATURE, //
        RMCXD_PORT(0x80653250, 0x8064EDC8, 0x806528BC, 0x80641568), //
        // clang-format off
        [](mkw::Net::RacePacketHandler* racePacketHandler
                ) -> mkw::Net::RacePacketHandler* {
            racePacketHandler->_008 = 0;
            racePacketHandler->_00C = 0;
            racePacketHandler->_010 = 0;
            racePacketHandler->_012 = 3000;

            mkw::Net::RacePacketHandler::ResetLocalLagFrameCount();

            return racePacketHandler;
        }
        // clang-format on
    ),
};

extern "C" {
__attribute__((used)) static void ProcessLocalLagFrames()
{
    using namespace mkw::Net;

    if (!NetController::Instance()->inVanillaMatch()) {
        return;
    }

    mkw::System::GameScene* gameScene = mkw::System::GameScene::Instance();
    if (!gameScene->involuntarilySkippedFrame()) {
        return;
    }

    RacePacketHandler::IncrementLocalLagFrameCount();
    if (!RacePacketHandler::ReachedLocalLagFrameThreshold()) {
        return;
    }

    wwfc::GPReport::ReportU32(
        "mkw_too_many_frames_dropped", RacePacketHandler::LocalLagFrameCount()
    );

    RacePacketHandler::ResetLocalLagFrameCount();
}
}

// Prevent this client from lagging rooms
WWFC_DEFINE_PATCH = {
    Patch::BranchWithCTR( //
        WWFC_PATCH_LEVEL_FEATURE, //
        RMCXD_PORT(0x80654CF8, 0x80650870, 0x80654364, 0x80643010), //
        ASM_LAMBDA(
            // clang-format off
            lwz       r0, 0x24(r1);
            mtlr      r0;
            addi      r1, r1, 0x20;

            b         ProcessLocalLagFrames;
            // clang-format on
        )
    ),
};

// Prevent clients from stalling rooms
WWFC_DEFINE_PATCH = {
    Patch::CallWithCTR( //
        WWFC_PATCH_LEVEL_FEATURE, //
        RMCXD_PORT(0x806579A0, 0x80653518, 0x8065700C, 0x80645CB8), //
        // clang-format off
        [](mkw::Net::NetController* netController) -> void {
            using namespace mkw::Net;

            netController->sendRacePacket();

            UserHandler::Instance()->calc();

            SelectHandler* selectHandler = SelectHandler::Instance();
            if (selectHandler) {
                selectHandler->processKicks();
            }
        }
        // clang-format on
    ),
};

// Fix a bug that leads to the rejection of one's item request without
// justification
WWFC_DEFINE_PATCH = {
    Patch::BranchWithCTR( //
        WWFC_PATCH_LEVEL_BUGFIX | WWFC_PATCH_LEVEL_PARITY, //
        RMCXD_PORT(0x8065C6C0, 0x8065D348, 0x8065BD2C, 0x8064A9D8), //
        // clang-format off
        [](mkw::Net::ItemHandler* itemHandler, u32 playerId,
           mkw::Item::ItemBox item) -> void {
            itemHandler->broadcastDecidedItem(playerId, item);
        }
        // clang-format on
    ),
};

// Reset the timer that is used to detect if clients are stalling the room
WWFC_DEFINE_PATCH = {
    Patch::CallWithCTR( //
        WWFC_PATCH_LEVEL_FEATURE, //
        RMCXD_PORT(0x8065FF34, 0x80657FF8, 0x8065F5A0, 0x8064E24C), //
        // clang-format off
        [](mkw::Net::SelectHandler* selectHandler,
           mkw::Net::NetController* netController) -> mkw::Net::SelectHandler* {
            using namespace mkw::Net;

            netController->_29B0 = 0;
            netController->_29B4 = 0;
            netController->_29B8 = 0;
            netController->_29BC = 0;

            SelectHandler::ResetKickTimer();

            return selectHandler;
        }
        // clang-format on
    ),
};

// Remove the 100cc engine class from vanilla matches
WWFC_DEFINE_PATCH = {
    Patch::CallWithCTR( //
        WWFC_PATCH_LEVEL_PARITY, //
        RMCXD_PORT(0x806613F8, 0x806594BC, 0x80660A64, 0x8064F710), //
        // clang-format off
        [](mkw::Util::Random* random) -> void {
            using namespace mkw::Net;

            SelectHandler* selectHandler = SelectHandler::Instance();
            if (NetController::Instance()->inVanillaMatch()) {
                selectHandler->decideEngineClassNo100cc(random);
            } else {
                selectHandler->decideEngineClass();
            }

            random->dt(random, -1);
        }
        // clang-format on
    ),
};

// Report information about the upcoming match to the server
WWFC_DEFINE_PATCH = {
    Patch::CallWithCTR( //
        WWFC_PATCH_LEVEL_FEATURE, //
        RMCXD_PORT(0x8066148C, 0x80659550, 0x80660AF8, 0x8064F7A4), //
        // clang-format off
        []() -> void {
            using namespace mkw::Net;

            SelectHandler* selectHandler = SelectHandler::Instance();
            selectHandler->decideCourse();
            selectHandler->initPlayerIdsToPlayerAids();

            SelectHandler::Packet::SelectedCourse selectedCourse =
                selectHandler->sendPacket().selectedCourse;
            SelectHandler::Packet::EngineClass engineClass =
                selectHandler->sendPacket().engineClass;

            wwfc::GPReport::ReportU32(
                "mkw_select_course", static_cast<u32>(selectedCourse)
            );
            wwfc::GPReport::ReportU32("mkw_select_cc", static_cast<u32>(engineClass));
        }
        // clang-format on
    ),
};

// Allow the "Open Host" feature to be enabled via the press of a button
WWFC_DEFINE_PATCH = {
    Patch::WritePointer( //
        WWFC_PATCH_LEVEL_FEATURE, //
        RMCXD_PORT(0x808B9008, 0x808BABF8, 0x808B8158, 0x808A7470), //
        FriendRoomPage_onActivate
    ),
};
WWFC_DEFINE_PATCH = {
    Patch::WritePointer( //
        WWFC_PATCH_LEVEL_FEATURE, //
        RMCXD_PORT(0x808B900C, 0x808BABFC, 0x808B815C, 0x808A7474), //
        FriendRoomPage_onDeactivate
    ),
};
WWFC_DEFINE_PATCH = {
    Patch::WritePointer( //
        WWFC_PATCH_LEVEL_FEATURE, //
        RMCXD_PORT(0x808B902C, 0x808BAC1C, 0x808B817C, 0x808A7494), //
        FriendRoomPage_onRefocus
    ),
};
WWFC_DEFINE_PATCH = {
    Patch::WritePointer( //
        WWFC_PATCH_LEVEL_FEATURE, //
        RMCXD_PORT(0x808BFE7C, 0x808B97CC, 0x808BEFCC, 0x808AE2EC), //
        WifiFriendMenu_onActivate
    ),
};
WWFC_DEFINE_PATCH = {
    Patch::WritePointer( //
        WWFC_PATCH_LEVEL_FEATURE, //
        RMCXD_PORT(0x808BFE80, 0x808B97D0, 0x808BEFD0, 0x808AE2F0), //
        WifiFriendMenu_onDeactivate
    ),
};
WWFC_DEFINE_PATCH = {
    Patch::WritePointer( //
        WWFC_PATCH_LEVEL_FEATURE, //
        RMCXD_PORT(0x808BFEA0, 0x808B97F0, 0x808BEFF0, 0x808AE310), //
        WifiFriendMenu_onRefocus
    ),
};

#endif

} // namespace wwfc::Feature

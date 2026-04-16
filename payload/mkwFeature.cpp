#if RMC

#  include "import/mkw/net/NetSelectHandler.hpp"
#  include "import/mkw/net/NetUserHandler.hpp"
#  include "import/mkw/ui/page/WifiMenuPage.hpp"
#  include "wwfcPatch.hpp"

namespace wwfc
{

using namespace mkw;

u32 NetManager::s_reportedAids          = 0x00000000;
u32 NetSelectHandler::s_kickTimerFrames = 0;

} // namespace wwfc

namespace wwfc::mkw::UI
{

const wchar_t* WifiMenuPage::s_messageOfTheDay = L"Welcome to\nWiiLink Wi-Fi Connection!";
wchar_t        WifiMenuPage::s_messageOfTheDayBuffer[256] = {};
bool           WifiMenuPage::s_hasSeenMessageOfTheDay     = false;

} // namespace wwfc::mkw::UI

namespace wwfc::mkw::Feature
{

static GameSpy::GPResult GetMessageOfTheDay(GameSpy::GPResult gpResult, const char* message)
{
    using namespace wwfc::mkw::UI;

    const char loginChallenge2Message[] = "\\lc\\2";
    if (std::strncmp(message, loginChallenge2Message, sizeof(loginChallenge2Message) - 1)) {
        return gpResult;
    }

    const char motdKey[] = "\\wl:motd\\";
    char       value[512];
    if (!GameSpy::gpiValueForKey(message, motdKey, value, sizeof(value))) {
        return gpResult;
    }

    wchar_t* messageOfTheDayBuffer = WifiMenuPage::MessageOfTheDayBuffer();
    s32 messageOfTheDayBufferSize  = static_cast<s32>(WifiMenuPage::MessageOfTheDayBufferSize());
    s32 messageOfTheDayLength      = DWC::DWC_Base64Decode(
        value, std::strlen(value), reinterpret_cast<char*>(messageOfTheDayBuffer),
        messageOfTheDayBufferSize
    );
    if (messageOfTheDayLength == -1 || messageOfTheDayLength == messageOfTheDayBufferSize) {
        return gpResult;
    }
    messageOfTheDayBuffer[messageOfTheDayLength / sizeof(wchar_t)] = L'\0';

    WifiMenuPage::SetMessageOfTheDay(messageOfTheDayBuffer);

    return gpResult;
}

// Get the Message Of The Day from the "Login Challenge 2" message
WWFC_DEFINE_PATCH = Patch::BranchWithCTR(
    WWFC_PATCH_LEVEL_FEATURE,
    RMCXD_PORT(0x80101074, 0x80100FD4, 0x80100F94, 0x801010EC, 0x80100E44),

    ASM_LAMBDA(
        ( : ASM_IMPORT(i, GetMessageOfTheDay)),
        // clang-format off
            mr        r4, r26;

            bl        _restgpr_26;
            lwz       r0, 0x2D4(r1);
            mtlr      r0;
            addi      r1, r1, 0x2D0;
            
            b         %[GetMessageOfTheDay];
        // clang-format on
    )
);

// Display the Message Of The Day when a client connects to the server
WWFC_DEFINE_PATCH = Patch::BranchWithCTR(
    WWFC_PATCH_LEVEL_FEATURE,
    RMCXD_PORT(0x8064BCD4, 0x806189C0, 0x8064B340, 0x80639FEC, 0x8064C208),

    ASM_LAMBDA(
        ( : ASM_IMPORT_AS(i, UI::WifiMenuPage_showMessageOfTheDay, ShowMessageOfTheDay)),
        // clang-format off
            mr        r3, r31;
            
            lwz       r31, 0x0C(r1);
            lwz       r0, 0x14(r1);
            mtlr      r0;
            addi      r1, r1, 0x10;
            
            b         %[ShowMessageOfTheDay];
        // clang-format on
    )
);

// Prevent clients from stalling rooms
WWFC_DEFINE_PATCH = 
    Patch::CallWithCTR( //
        WWFC_PATCH_LEVEL_FEATURE, //
        RMCXD_PORT(0x806579A0, 0x80653518, 0x8065700C, 0x80645CB8, 0x80657EE4),
        
        [](mkw::NetManager* netController) -> void {
    netController->sendRacePacket();

    NetUserHandler::Instance()->calc();

    NetSelectHandler* selectHandler = NetSelectHandler::Instance();
    if (selectHandler) {
        selectHandler->processKicks();
    }
}
    );

// Clear the flag that indicates whether an aid was reported to the server
// when closing the connection to them.
WWFC_DEFINE_PATCH = 
    Patch::BranchWithCTR( //
        WWFC_PATCH_LEVEL_FEATURE, //
        RMCXD_PORT(0x806588C8, 0x80654440, 0x80657F34, 0x80646BE0, 0x80658E0C), 

        ASM_LAMBDA((:ASM_IMPORT_AS(i, mkw::NetManager::ClearReportedAid, ClearReportedAid)),
            // clang-format off
            mr        r3, r28;

            lwz       r28, 0x10(r1);
            mtlr      r0;
            addi      r1, r1, 0x20;

            b         %[ClearReportedAid];
            // clang-format on
        )
    );

// Reset the timer that is used to detect if clients are stalling the room
WWFC_DEFINE_PATCH = 
    Patch::CallWithCTR( //
        WWFC_PATCH_LEVEL_FEATURE, //
        RMCXD_PORT(0x8065FF34, 0x80657FF8, 0x8065F5A0, 0x8064E24C, 0x80660478), 

        [](mkw::NetSelectHandler* selectHandler,
           mkw::NetManager* netController) -> mkw::NetSelectHandler* {
    netController->_29B0 = 0;
    netController->_29B4 = 0;
    netController->_29B8 = 0;
    netController->_29BC = 0;

    NetSelectHandler::ResetKickTimer();

    return selectHandler;
}
    );

// Report information about the upcoming match to the server
WWFC_DEFINE_PATCH = 
    Patch::CallWithCTR( //
        WWFC_PATCH_LEVEL_FEATURE, //
        RMCXD_PORT(0x8066148C, 0x80659550, 0x80660AF8, 0x8064F7A4, 0x806619D0),

        []() -> void {
    NetSelectHandler* selectHandler = NetSelectHandler::Instance();
    selectHandler->decideCourse();
    selectHandler->initPlayerIdsToPlayerAids();


    wwfc::GPReport::ReportU32(
        "wl:mkw_select_course", static_cast<u32>(selectHandler->getSendPacket().selectedCourse)
    );
    wwfc::GPReport::ReportU32(
        "wl:mkw_select_cc", static_cast<u32>(selectHandler->getSendPacket().engineClass)
    );
}
    );

} // namespace wwfc::mkw::Feature

#endif // RMC
#include "import/dwc.h"
#include "import/gamespy.h"
#include "import/mkwNetSelectHandler.hpp"
#include "import/mkwUIPage.hpp"
#include "import/mkwUtil.hpp"
#include <cstring>

namespace mkw::UI
{

#if RMC

const wchar_t* WifiMenuPage::s_messageOfTheDay =
    L"Welcome to\nWiiLink Wi-Fi Connection!";
wchar_t WifiMenuPage::s_messageOfTheDayBuffer[256] = {};
bool WifiMenuPage::s_hasSeenMessageOfTheDay = false;

#endif

} // namespace mkw::UI

namespace wwfc::Feature
{

#if RMC

static void DecideEngineClass(
    mkw::Net::SelectHandler* selectHandler, mkw::Util::Random* random
)
{
    using namespace mkw::Net;

    SelectHandler::Packet& sendPacket = selectHandler->sendPacket();

    if (random->nextInt(100) < 65) {
        sendPacket.engineClass = SelectHandler::Packet::EngineClass::e150cc;
    } else {
        sendPacket.engineClass =
            SelectHandler::Packet::EngineClass::eMirrorMode;
    }
}

// Remove the 100cc engine class from vanilla matches
WWFC_DEFINE_PATCH = {
    Patch::CallWithCTR( //
        WWFC_PATCH_LEVEL_PARITY, //
        RMCXD_PORT(0x806613F8, 0x806594BC, 0x80660A64, 0x8064F710), //
        [](mkw::Util::Random* random) -> void {
            using namespace mkw::Net;

            SelectHandler* selectHandler = SelectHandler::Instance();
            if (RKNetController::Instance()->inVanillaMatch()) {
                DecideEngineClass(selectHandler, random);
            } else {
                selectHandler->decideEngineClass();
            }

            random->dt(random, -1);
        }
    ),
};

extern "C" {

__attribute__((__used__)) static GameSpy::GPResult
GetMessageOfTheDay(const char* message)
{
    using namespace mkw::UI;

    const char loginChallenge2Message[] = "\\lc\\2";
    if (strncmp(
            message, loginChallenge2Message, sizeof(loginChallenge2Message) - 1
        )) {
        return GameSpy::GPNoError;
    }

    const char motdKey[] = "\\wwfc_motd\\";
    char value[512];
    if (!GameSpy::gpiValueForKey(message, motdKey, value, sizeof(value))) {
        return GameSpy::GPNoError;
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
        return GameSpy::GPNoError;
    }
    messageOfTheDayBuffer[messageOfTheDayLength / sizeof(wchar_t)] = L'\0';

    WifiMenuPage::SetMessageOfTheDay(messageOfTheDayBuffer);

    return GameSpy::GPNoError;
}
}

// Get the "Message Of The Day" from the "Login Challenge" message
WWFC_DEFINE_PATCH = {
    Patch::BranchWithCTR(
        WWFC_PATCH_LEVEL_FEATURE,
        RMCXD_PORT(0x80101074, 0x80100FD4, 0x80100F94, 0x801010EC), //
        ASM_LAMBDA(
            // clang-format off
            mr        r3, r26;

            bl        _restgpr_26;
            lwz       r0, 0x2D4(r1);
            mtlr      r0;
            addi      r1, r1, 0x2D0;
            
            b         GetMessageOfTheDay;
            // clang-format on
        )
    ),
};

extern "C" {

__attribute__((__used__)) static void
ShowMessageOfTheDay(mkw::UI::WifiMenuPage* wifiMenuPage)
{
    using namespace mkw::UI;

    if (WifiMenuPage::HasSeenMessageOfTheDay()) {
        return;
    }

    wifiMenuPage->showMessageOfTheDay();

    WifiMenuPage::SeenMessageOfTheDay();
}
}

// Display a "Message Of The Day" when a client connects to the server
WWFC_DEFINE_PATCH = {
    Patch::BranchWithCTR(
        WWFC_PATCH_LEVEL_FEATURE,
        RMCXD_PORT(0x8064BCD4, 0x806189C0, 0x8064B340, 0x80639FEC), //
        ASM_LAMBDA(
            // clang-format off
            mr        r3, r31;
            
            lwz       r31, 0x0C(r1);
            lwz       r0, 0x14(r1);
            mtlr      r0;
            addi      r1, r1, 0x10;
            
            b         ShowMessageOfTheDay;
            // clang-format on
        )
    ),
};

#endif

} // namespace wwfc::Feature

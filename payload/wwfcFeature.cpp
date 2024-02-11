#include "import/mkwNetSelectHandler.hpp"
#include "import/mkwUIPage.hpp"
#include "import/mkwUtil.hpp"

namespace mkw::UI
{

#if RMC

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

__attribute__((__used__)) static void
ShowMessageOfTheDay(mkw::UI::WifiMenuPage* wifiMenuPage)
{
    if (mkw::UI::WifiMenuPage::HasSeenMessageOfTheDay()) {
        return;
    }

    wifiMenuPage->showMessageOfTheDay();

    mkw::UI::WifiMenuPage::SeenMessageOfTheDay();
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

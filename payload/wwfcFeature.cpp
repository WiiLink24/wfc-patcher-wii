#include "import/mkwNet.hpp"
#include "import/mkwUtil.hpp"

namespace wwfc::Feature
{

#if RMC

static void DecideEngineClass(mkw::Net::SELECTHandler* selectHandler)
{
    using namespace mkw::Net;

    SELECTHandler::SELECTPacket* sendPacket = selectHandler->sendPacket();

    if (__builtin_ppc_mftb() & 1) {
        sendPacket->engineClass =
            SELECTHandler::SELECTPacket::EngineClass::e150cc;
    } else {
        sendPacket->engineClass =
            SELECTHandler::SELECTPacket::EngineClass::eMirrorMode;
    }
}

// Remove the 100cc engine class from Worldwide Versus Races
WWFC_DEFINE_PATCH = {
    Patch::CallWithCTR( //
        WWFC_PATCH_LEVEL_FEATURE, //
        RMCXD_PORT(0x806613F8, 0x806594BC, 0x80660A64, 0x8064F710), //
        [](mkw::Util::Random* random) -> void {
            random->dt(random, -1);

            using namespace mkw::Net;

            RKNetController::MatchType matchType =
                RKNetController::Instance()->matchType();
            SELECTHandler* selectHandler = SELECTHandler::Instance();

            if (matchType == RKNetController::MatchType::WorldwideVersusRace) {
                DecideEngineClass(selectHandler);
            } else {
                selectHandler->decideEngineClass();
            }
        }
    ),
};

#endif

} // namespace wwfc::Feature

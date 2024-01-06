#include "import/mkwNet.hpp"
#include "import/mkwUtil.hpp"

namespace wwfc::Feature
{

#if RMC

static void DecideEngineClass(
    mkw::Net::SELECTHandler* selectHandler, mkw::Util::Random* random
)
{
    using namespace mkw::Net;

    SELECTHandler::Packet& sendPacket = selectHandler->sendPacket();

    if (random->nextInt(100) < 65) {
        sendPacket.engineClass = SELECTHandler::Packet::EngineClass::e150cc;
    } else {
        sendPacket.engineClass =
            SELECTHandler::Packet::EngineClass::eMirrorMode;
    }
}

// Remove the 100cc engine class from Worldwide Versus Races
WWFC_DEFINE_PATCH = {
    Patch::CallWithCTR( //
        WWFC_PATCH_LEVEL_PARITY, //
        RMCXD_PORT(0x806613F8, 0x806594BC, 0x80660A64, 0x8064F710), //
        [](mkw::Util::Random* random) -> void {
            using namespace mkw::Net;

            SELECTHandler* selectHandler = SELECTHandler::Instance();
            if (RKNetController::Instance()->inVanillaMatch()) {
                DecideEngineClass(selectHandler, random);
            } else {
                selectHandler->decideEngineClass();
            }

            random->dt(random, -1);
        }
    ),
};

#endif

} // namespace wwfc::Feature

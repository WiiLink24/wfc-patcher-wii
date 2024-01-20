#include "wwfcGPReport.hpp"
#include "import/dwc.h"
#include "import/gamespy.h"
#include "wwfcLog.hpp"

namespace wwfc::GPReport
{

#if RMC

void ReportUser(mkw::Net::UserHandler::Packet* packet)
{
    auto connection = DWC::stpMatchCnt->connection;
    if (connection == nullptr) {
        return;
    }

    auto iconnection = reinterpret_cast<GameSpy::GPIConnection*>(*connection);

    char b64UserPacket[0x400];
    s32 b64Len = DWC::DWC_Base64Encode(
        packet, sizeof(mkw::Net::UserHandler::Packet), b64UserPacket,
        sizeof(b64UserPacket)
    );
    if (b64Len < 0 || b64Len >= 0x400) {
        LOG_ERROR("Could not base64 encode the User packet");
        return;
    }

    // Add null terminator
    b64UserPacket[b64Len] = '\0';

    GameSpy::gpiAppendStringToBuffer(
        connection, &iconnection->outputBuffer, "\\wwfc_report\\\\mkw_user\\"
    );
    GameSpy::gpiAppendStringToBuffer(
        connection, &iconnection->outputBuffer, b64UserPacket
    );
    GameSpy::gpiAppendStringToBuffer(
        connection, &iconnection->outputBuffer, "\\final\\"
    );
}

#endif

} // namespace wwfc::GPReport
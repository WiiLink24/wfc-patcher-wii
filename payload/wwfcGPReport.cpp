#include "wwfcGPReport.hpp"
#include "import/dwc.h"
#include "import/gamespy.h"
#include "wwfcLibC.hpp"
#include "wwfcLog.hpp"

namespace wwfc::GPReport
{

#if RMC

void Report(const char* key, const char* string)
{
    auto connection = DWC::stpMatchCnt->connection;
    if (connection == nullptr) {
        return;
    }

    auto iconnection = reinterpret_cast<GameSpy::GPIConnection*>(*connection);

    GameSpy::gpiAppendStringToBuffer(
        connection, &iconnection->outputBuffer, "\\wl:report\\\\"
    );
    GameSpy::gpiAppendStringToBuffer(
        connection, &iconnection->outputBuffer, key
    );
    GameSpy::gpiAppendStringToBuffer(
        connection, &iconnection->outputBuffer, "\\"
    );
    GameSpy::gpiAppendStringToBuffer(
        connection, &iconnection->outputBuffer, string
    );
    GameSpy::gpiAppendStringToBuffer(
        connection, &iconnection->outputBuffer, "\\final\\"
    );
}

void ReportU32(const char* key, u32 uint)
{
    char buffer[sizeof("4294967295")];

    if (snprintf(buffer, sizeof(buffer), "%lu", uint) < 0) {
        return;
    }

    Report(key, buffer);
}

void ReportB64Encode(const char* key, const void* data, size_t dataSize)
{
    char b64Data[0x400];

    s32 b64Size =
        DWC::DWC_Base64Encode(data, dataSize, b64Data, sizeof(b64Data));
    if (b64Size == -1 || b64Size == sizeof(b64Data)) {
        LOG_ERROR(
            "Could not fit the base64-encoded data into the provided buffer!"
        );
        return;
    }
    b64Data[b64Size] = '\0';

    Report(key, b64Data);
}

#endif

} // namespace wwfc::GPReport
#include "wwfcPatch.hpp"
#include "wwfcPayload.hpp"
#include "wwfcUtil.h"

namespace wwfc::Login
{

WWFC_DEFINE_PATCH = {Patch::CallWithCTR(
    WWFC_PATCH_LEVEL_CRITICAL, //
    ADDRESS_PATCH_GPISENDLOGIN, //
    ASM_LAMBDA(
        // clang-format off
        mr      r3, r28;
        addi    r4, r30, 0x210;
        b       SendExtendedLogin;
        // clang-format on
    )
)};

int SendExtendedLogin(void* connection, void* outputBuffer) //
    asm("SendExtendedLogin");

int SendExtendedLogin(void* connection, void* outputBuffer)
{
    LONGCALL int gpiAppendStringToBuffer( //
        void* connection, void* outputBuffer, const char* buffer
    ) AT(ADDRESS_gpiAppendStringToBuffer);

    LONGCALL int gpiAppendIntToBuffer( //
        void* connection, void* outputBuffer, int num
    ) AT(ADDRESS_gpiAppendIntToBuffer);

    gpiAppendStringToBuffer(connection, outputBuffer, "\\payload_ver\\");
    gpiAppendIntToBuffer(
        connection, outputBuffer, Payload::Header.info.version
    );

    gpiAppendStringToBuffer(connection, outputBuffer, "\\final\\");

    return 0;
}

} // namespace wwfc::Login

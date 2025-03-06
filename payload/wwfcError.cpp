#include "import/dwc.h"
#include "import/gamespy.h"
#include "import/mkw/ui/ui.hpp"
#include "wwfcPatch.hpp"
#include <cstring>

namespace wwfc::Error
{

void HandleWWFCErrorMessage( //
    GameSpy::GPConnection* connection, GameSpy::GPResult result, int isFatal,
    const char* command
) asm("HandleWWFCErrorMessage");

WWFC_DEFINE_PATCH = {Patch::CallWithCTR(
    WWFC_PATCH_LEVEL_SUPPORT | WWFC_PATCH_LEVEL_FEATURE, //
    ADDRESS_PATCH_gpiCheckForError, // 0x80108F78
    ASM_LAMBDA(
// clang-format off
#if TYPE_gpiCheckForError == 0
        mr      r6, r29;
#else
        mr      r6, r28;
#endif
        b       HandleWWFCErrorMessage;
        // clang-format on
    )
)};

static int s_wwfcErrorCode = 0;
#if RMC
static wchar_t s_wwfcErrorMsg[256] = {};
#endif

void HandleWWFCErrorMessage(
    GameSpy::GPConnection* connection, GameSpy::GPResult result, int isFatal,
    const char* command
)
{
    LONGCALL int atoi( //
        const char* str
    ) AT(ADDRESS_atoi);

    char value[512];
    int error = 0;
    if (GameSpy::gpiValueForKey(
            command, "\\wl:err\\", value, sizeof(value) - 1
        ) != GameSpy::GPIFalse) {
        error = atoi(value);
        if (error != 0) {
            s_wwfcErrorCode = error;
        }
    }

    GameSpy::gpiCallErrorCallback(
        connection, result,
        isFatal != 0 ? GameSpy::GPEnum::GP_FATAL : GameSpy::GPEnum::GP_NON_FATAL
    );

#if RMC
    if (error == 0) {
        return;
    }

    if (GameSpy::gpiValueForKey(
            command, "\\wl:errmsg\\", value, sizeof(value)
        ) == GameSpy::GPIFalse) {
        return;
    }

    s32 errorMessageLength = DWC::DWC_Base64Decode(
        value, strlen(value), reinterpret_cast<char*>(s_wwfcErrorMsg),
        sizeof(s_wwfcErrorMsg)
    );
    if (errorMessageLength == -1 ||
        errorMessageLength == sizeof(s_wwfcErrorMsg)) {
        return;
    }
    s_wwfcErrorMsg[errorMessageLength / sizeof(wchar_t)] = L'\0';
#endif
}

#if RMC
s32 ExplainWWFCError( //
    s32 error, mkw::UI::FormatParam* formatParam
) asm("ExplainWWFCError");

WWFC_DEFINE_PATCH = {Patch::CallWithCTR(
    WWFC_PATCH_LEVEL_SUPPORT | WWFC_PATCH_LEVEL_FEATURE, //
    RMCXD_PORT(0x80649C68, 0x80617458, 0x806492D4, 0x80637F80), //
    ASM_LAMBDA(
        // clang-format off
        mflr    r0;
        stw     r0, 0x8(r1);

        lwz     r3, 0x18C(r31);
        addi    r4, r1, 0x10;
        bl      ExplainWWFCError;
        mr      r4, r3;

        lwz     r0, 0x8(r1);
        mtlr    r0;
        blr;
        // clang-format on
    )
)};

s32 ExplainWWFCError(s32 error, mkw::UI::FormatParam* formatParam)
{
    LONGCALL s32 ExplainDWCError( //
        s32 errorCode
    ) AT(RMCXD_PORT(0x80649DA4, 0x80617594, 0x80649410, 0x806380BC));

    if (s_wwfcErrorCode != 0) {
        error = s_wwfcErrorCode;
        s_wwfcErrorCode = 0;

        formatParam->strings[0] = s_wwfcErrorMsg;
        return 6602;
    }

    formatParam->numbers[0] = error;
    return ExplainDWCError(error);
}

#else

u32 SetWWFCErrorCode( //
    u32 ret
) asm("SetWWFCErrorCode");

// Display the error code for non-Mario Kart Wii
WWFC_DEFINE_PATCH = {Patch::BranchWithCTR(
    WWFC_PATCH_LEVEL_SUPPORT | WWFC_PATCH_LEVEL_FEATURE, //
    ADDRESS_PATCH_DWCi_HandleGPError, // 0x800D2E3C
    ASM_LAMBDA(
        // clang-format off
        lwz     r29, 0x14(r1);
        mtlr    r0;
        addi    r1, r1, 0x20;
        b       SetWWFCErrorCode;
        // clang-format on
    )
)};

u32 SetWWFCErrorCode(u32 ret)
{
    if (s_wwfcErrorCode != 0) {
        DWC::DWCi_SetError(6, -s_wwfcErrorCode);
        s_wwfcErrorCode = 0;
    }

    return ret;
}

#endif

} // namespace wwfc::Error
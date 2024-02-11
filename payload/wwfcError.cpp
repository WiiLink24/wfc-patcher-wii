#include "import/gamespy.h"
#include "import/mkwUI.hpp"
#include "wwfcPatch.hpp"

namespace wwfc::Error
{

#if RMC

char* HandleWWFCErrorMessage( //
    char* strstrResult, const char* command
) asm("HandleWWFCErrorMessage");

s32 ExplainWWFCError( //
    s32 error, mkw::UI::FormatParam* formatParam
) asm("ExplainWWFCError");

WWFC_DEFINE_PATCH = {
    Patch::CallWithCTR(
        WWFC_PATCH_LEVEL_SUPPORT | WWFC_PATCH_LEVEL_FEATURE, //
        RMCXD_PORT(0x80108F64, 0x80108EC4, 0x80108E84, 0x80108FDC), //
        ASM_LAMBDA(
            // clang-format off
            mflr    r31;

            mr      r4, r29;
            bl      HandleWWFCErrorMessage;

            neg     r0, r3;
            li      r4, 4;
            or      r0, r0, r3;
            mr      r3, r28;
            mtlr    r31;
            blr;
            // clang-format on
        )
    ),
    Patch::CallWithCTR(
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
    )
};

static int s_wwfcErrorCode = 0;
static wchar_t s_wwfcErrorMsg[256] = {};

char* HandleWWFCErrorMessage(char* strstrResult, const char* command)
{
    LONGCALL GameSpy::GPIBool gpiValueForKey( //
        const char* command, const char* key, char* value, int len
    ) AT(RMCXD_PORT(0x80108FB4, 0x80108F14, 0x80108ED4, 0x8010902C));

    LONGCALL int atoi( //
        const char* str
    ) AT(RMCXD_PORT(0x8001543C, 0x800148DC, 0x80015360, 0x800154A4));

    if (strstrResult == nullptr) {
        return nullptr;
    }

    char value[256];
    if (gpiValueForKey(command, "\\wwfc_err\\", value, sizeof(value)) ==
        GameSpy::GPIFalse) {
        return strstrResult;
    }

    int error = atoi(value);
    if (error == 0) {
        return strstrResult;
    }

    if (gpiValueForKey(command, "\\wwfc_errmsg\\", value, sizeof(value)) ==
        GameSpy::GPIFalse) {
        return strstrResult;
    }

    // TODO: Convert UTF-8 to UTF-16 properly
    size_t n = 0;
    for (;
         value[n] != '\0' && n < (sizeof(s_wwfcErrorMsg) / sizeof(wchar_t)) - 1;
         n++) {
        s_wwfcErrorMsg[n] = value[n];
    }
    s_wwfcErrorMsg[n] = '\0';

    s_wwfcErrorCode = error;

    return strstrResult;
}

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

    // TODO: Display custom messages for generic errors as well
    formatParam->numbers[0] = error;
    return ExplainDWCError(error);
}

#endif

} // namespace wwfc::Error
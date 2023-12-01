#include "wwfcLog.hpp"
#include "wwfcPatch.hpp"
#include <cstring>
#include <import/revolution.h>

namespace wwfc::Support
{

static_assert(sizeof(WWFC_DOMAIN) <= sizeof("nintendowifi.net"));

// Unpatch the auth response hook
WWFC_DEFINE_PATCH = {Patch::Write(
    WWFC_PATCH_LEVEL_SUPPORT, //
    ADDRESS_AUTH_HANDLERESP_HOOK, //
    (u32[]){AUTH_HANDLERESP_UNPATCH}
)};

static const char* FixHostname(const char* name, char* out)
{
    if (std::strlen(name) > 256 - sizeof(WWFC_DOMAIN)) {
        // Hostname is too big to replace
        return name;
    }

    for (u32 i = 0; name[i] != '\0'; i++) {
        auto replaceDomain = [&](const char* check, u32 checkSize,
                                 const char* replace, u32 replaceSize) {
            if (std::strncmp(name + i, check, checkSize) != 0) {
                return false;
            }

            // Replace with the custom hostname
            std::memcpy(out, name, i);
            std::memcpy(out + i, replace, replaceSize);
            std::strcpy(out + i + replaceSize, name + i + checkSize);
            LOG_INFO_FMT("Hostname: %s -> %s", name, out);
            return true;
        };

        // Replace nintendowifi.net with the custom name
        if (replaceDomain(
                "nintendowifi.net", sizeof("nintendowifi.net") - 1, WWFC_DOMAIN,
                sizeof(WWFC_DOMAIN) - 1
            )) {
            return out;
        }

        // Redirect gamespy.com to the "gs" subdomain, required for some games
        if (replaceDomain(
                "gamespy.com", sizeof("gamespy.com"), "gs." WWFC_DOMAIN,
                sizeof("gs." WWFC_DOMAIN) - 1
            )) {
            return out;
        }

        // WWFC patch over an already Wiimmfi patched game
        if (replaceDomain(
                "wiimmfi.de", sizeof("wiimmfi.de") - 1, WWFC_DOMAIN,
                sizeof(WWFC_DOMAIN) - 1
            )) {
            return out;
        }
    }

    // No custom hostname
    return name;
}

WWFC_DEFINE_CTR_STUB( //
    ADDRESS_gethostbyname + 0x10, //
    void* gethostbyname(const char* name),

    // clang-format off
    stwu    sp, -0x30(sp);
    mflr    r0;
    stw     r0, 0x34(sp);
    addi    r11, sp, 0x30;
    // clang-format on
)

WWFC_DEFINE_PATCH = {Patch::BranchWithCTR(
    WWFC_PATCH_LEVEL_SUPPORT, //
    ADDRESS_gethostbyname, //
    [](char* name) -> void* {
        char fixedName[256];
        return gethostbyname(FixHostname(name, fixedName));
    }
)};

WWFC_DEFINE_CTR_STUB( //
    ADDRESS_SOInetAtoN + 0x10, //
    u32 SOInetAtoN(const char* name, u8* param_2),

    // clang-format off
    stwu    sp, -0x30(sp);
    mflr    r0;
    stw     r0, 0x34(sp);
    addi    r11, sp, 0x30;
    // clang-format on
)

WWFC_DEFINE_PATCH = {Patch::BranchWithCTR(
    WWFC_PATCH_LEVEL_SUPPORT, //
    ADDRESS_SOInetAtoN, //
    [](const char* name, u8* param_2) -> u32 {
        char fixedName[256];
        return SOInetAtoN(FixHostname(name, fixedName), param_2);
    }
)};

void ChangeAuthURL()
{
    *(const char**) ADDRESS_NASWII_AC_URL_POINTER =
        "http://naswii." WWFC_DOMAIN "/ac";

#if ADDRESS_NASWII_PR_URL_POINTER
    *(const char**) ADDRESS_NASWII_PR_URL_POINTER =
        "http://naswii." WWFC_DOMAIN "/pr";
#endif
}

#if ADDRESS_GHIPARSEURL_HTTPS_PATCH
// Disable SSL in GameSpy HTTP
WWFC_DEFINE_PATCH = {Patch::WriteASM(
    WWFC_PATCH_LEVEL_SUPPORT, //
    ADDRESS_GHIPARSEURL_HTTPS_PATCH, //
    1, ASM_LAMBDA(li r0, 0)
)};
#endif

} // namespace wwfc::Support

#include "import/revolution.h"
#include "wwfcLog.hpp"
#include "wwfcPatch.hpp"

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
            WWFC_LOG_INFO_FMT("Hostname: %s -> %s", name, out);
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

WWFC_DEFINE_CTR_STUB( //
    ADDRESS_SOGetAddrInfo + 0x10, //
    u32 SOGetAddrInfo(const char* name, u32 param_2, u32 param_3, u32 param_4),

    // clang-format off
    stwu    sp, -0x40(sp);
    mflr    r0;
    stw     r0, 0x44(sp);
    addi    r11, sp, 0x40;
    // clang-format on
)

WWFC_DEFINE_PATCH = {Patch::BranchWithCTR(
    WWFC_PATCH_LEVEL_SUPPORT, //
    ADDRESS_SOGetAddrInfo, //
    [](const char* name, u32 param_2, u32 param_3, u32 param_4) -> u32 {
        char fixedName[256];
        return SOGetAddrInfo(
            FixHostname(name, fixedName), param_2, param_3, param_4
        );
    }
)};

// Disable SSL in NHTTP
WWFC_DEFINE_PATCH = {
    Patch::WriteASM(
        WWFC_PATCH_LEVEL_SUPPORT, //
        ADDRESS_NHTTP_HTTPS_PORT_PATCH, //
        1, ASM_LAMBDA(li r0, 80)
    ),
    Patch::WriteASM(
        WWFC_PATCH_LEVEL_SUPPORT, //
        ADDRESS_NHTTPi_SocSSLConnect, //
        2, ASM_LAMBDA(li r3, 0; blr)
    ),
};

#if ADDRESS_GHIPARSEURL_HTTPS_PATCH

// Disable SSL in GameSpy HTTP
WWFC_DEFINE_PATCH = {Patch::WriteASM(
    WWFC_PATCH_LEVEL_SUPPORT, //
    ADDRESS_GHIPARSEURL_HTTPS_PATCH, //
    1, ASM_LAMBDA(li r0, 0)
)};

#endif

void ChangeAuthURL()
{
    *reinterpret_cast<const char**>(ADDRESS_NASWII_AC_URL_POINTER) =
        "http://nas." WWFC_DOMAIN "/ac";
}

} // namespace wwfc::Support

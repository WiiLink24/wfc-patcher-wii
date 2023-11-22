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

static void FixHostname(char* name)
{
    for (u32 i = 0; name[i] != '\0'; i++) {
        if (std::strncmp(
                name + i, "nintendowifi.net", sizeof("nintendowifi.net") - 1
            ) == 0) {
            OSReport("[WWFC] Hostname: %s", name);

            // Replace nintendowifi.net with the custom domain
            std::memcpy(name + i, WWFC_DOMAIN, sizeof(WWFC_DOMAIN) - 1);
            if (sizeof(WWFC_DOMAIN) < sizeof("nintendowifi.net")) {
                std::strcpy(
                    name + i + sizeof(WWFC_DOMAIN) - 1,
                    name + i + sizeof("nintendowifi.net") - 1
                );
            }

            OSReport(" -> %s\n", name);
            break;
        }

        if (std::strncmp(name + i, "gamespy.com", sizeof("gamespy.com") - 1) ==
            0) {
            OSReport("[WWFC] Hostname: %s", name);

            // Replace gamespy.com with the custom domain
            std::memcpy(
                name + i, "gs." WWFC_DOMAIN, sizeof("gs." WWFC_DOMAIN) - 1
            );
            if (sizeof("gs." WWFC_DOMAIN) < sizeof("gamespy.com")) {
                std::strcpy(
                    name + i + sizeof("gs." WWFC_DOMAIN) - 1,
                    name + i + sizeof("gamespy.com") - 1
                );
            }

            OSReport(" -> %s\n", name);
            break;
        }
    }
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
        FixHostname(name);
        return gethostbyname(name);
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
    [](char* name, u8* param_2) -> u32 {
        FixHostname(name);
        return SOInetAtoN(name, param_2);
    }
)};

void ChangeAuthURL()
{
    *(const char**) ADDRESS_NASWII_AC_URL_POINTER =
        "http://naswii." WWFC_DOMAIN "/ac";
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

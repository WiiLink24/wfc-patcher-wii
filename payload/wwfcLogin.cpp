#include "import/dwc.h"
#include "import/gamespy.h"
#include "import/revolution.h"
#include "wwfcHostPlatform.hpp"
#include "wwfcLog.hpp"
#include "wwfcPatch.hpp"
#include "wwfcPayload.hpp"
#include <cstring>

namespace wwfc::Login
{

int gpiSendLoginHook( //
    GameSpy::GPConnection* connection, GameSpy::GPIConnectData* data,
    GameSpy::GPIBuffer* outputBuffer
) asm("gpiSendLoginHook");

int gpiAddLocalInfoHook( //
    GameSpy::GPConnection* connection, GameSpy::GPIBuffer* outputBuffer
) asm("gpiAddLocalInfoHook");

static void SendExtendedLogin( //
    GameSpy::GPConnection* connection, const char* authToken,
    GameSpy::GPIBuffer* outputBuffer, bool sendProfileId
);

static void SendAuthTokenSignature( //
    GameSpy::GPConnection* connection, const char* authToken,
    GameSpy::GPIBuffer* outputBuffer, s32 esFd
);

static bool g_sendExLogin = false;

void Init()
{
    if (DWC::stpLoginCnt != nullptr && DWC::stpLoginCnt->state == 5) {
        g_sendExLogin = true;
    }
}

WWFC_DEFINE_PATCH = {
    Patch::CallWithCTR(
        WWFC_PATCH_LEVEL_CRITICAL, //
        ADDRESS_PATCH_GPISENDLOGIN, // 0x801007B0
        ASM_LAMBDA(
            // clang-format off
            mr      r3, r28;
            mr      r4, r29;
            addi    r5, r30, 0x210;
            b       gpiSendLoginHook;
            // clang-format on
        )
    ),
    Patch::CallWithCTR(
        // For the DNS patch method
        WWFC_PATCH_LEVEL_SUPPORT,
        ADDRESS_PATCH_GPIADDLOCALINFO, // 0x801021C0
        ASM_LAMBDA(
            // clang-format off
            mr      r3, r28;
            mr      r4, r29;
            mflr    r31;
            bl      gpiAddLocalInfoHook;
            mtlr    r31;
            lwz     r31, 0x1C(sp);
            lwz     r30, 0x18(sp);
            lwz     r29, 0x14(sp);
            blr;
            // clang-format on
        )
    ),
};

int gpiSendLoginHook(
    GameSpy::GPConnection* connection, GameSpy::GPIConnectData* data,
    GameSpy::GPIBuffer* outputBuffer
)
{
    SendExtendedLogin(connection, data->authtoken, outputBuffer, true);
    return 0;
}

int gpiAddLocalInfoHook(
    GameSpy::GPConnection* connection, GameSpy::GPIBuffer* outputBuffer
)
{
    if (g_sendExLogin) {
        GameSpy::gpiAppendStringToBuffer(
            connection, outputBuffer, "\\wl:exlogin\\"
        );

        SendExtendedLogin(
            connection, DWC::s_auth_result.authToken, outputBuffer, false
        );
    }
    return 0;
}

void SendExtendedLogin(
    GameSpy::GPConnection* connection, const char* authToken,
    GameSpy::GPIBuffer* outputBuffer, bool sendProfileId
)
{
    g_sendExLogin = false;

    if (sendProfileId) {
        DWC::DWCUserData* userData = DWC::DWCi_GetUserData();
        if (userData != nullptr && userData->profileId != 0) {
            GameSpy::gpiAppendStringToBuffer(
                connection, outputBuffer, "\\profileid\\"
            );
            GameSpy::gpiAppendIntToBuffer(
                connection, outputBuffer, userData->profileId
            );
        }
    }

    GameSpy::gpiAppendStringToBuffer(
        connection, outputBuffer, "\\wl:ver\\"
    );
    GameSpy::gpiAppendIntToBuffer(
        connection, outputBuffer, Payload::Header.info.version
    );

    if (authToken[0] != '\0') {
        s32 fd = RVL::IOS_Open("/dev/es", 0);
        bool usingEspHandle = false;
        bool interruptsEnabled;

        extern s32 EspFd AT(ADDRESS_ESP_FD);

        // ES_MAX_OPEN
        if (ADDRESS_ESP_FD != 0 && fd == -1016) {
            // I don't like this because we don't own this handle, but it seems
            // like somewhere on Wii U VC (the eShop Wii games) an ES handle is
            // opened and never closed

            interruptsEnabled = RVL::OSDisableInterrupts();

            // Steal the handle from the ESP library (and set the handle to -1
            // so it can't close it while we're using it)
            fd = EspFd;
            EspFd = -1;

            usingEspHandle = true;
        }

        if (fd >= 0) {
            SendAuthTokenSignature(connection, authToken, outputBuffer, fd);

            if (!usingEspHandle) {
                RVL::IOS_Close(fd);
            }
        } else {
            LOG_ERROR_FMT("Failed to open ES: %d", fd);
        }

        if (usingEspHandle) {
            EspFd = fd;
            RVL::OSRestoreInterrupts(interruptsEnabled);
        }
    }

    // TODO: Add more detailed information
    GameSpy::gpiAppendStringToBuffer(connection, outputBuffer, "\\wl:host\\");
    GameSpy::gpiAppendStringToBuffer(
        connection, outputBuffer, HostPlatform::IsDolphin() ? "Dolphin" : "Wii"
    );

    GameSpy::gpiAppendStringToBuffer(connection, outputBuffer, "\\final\\");
}

static u64 DecodeUintString(const char* str, u32 bitCount)
{
    u64 value = 0;

    for (u32 n = 0; n < (bitCount >> 2); n++) {
        u8 nybble = 0;
        if (str[n] >= '0' && str[n] <= '9') {
            nybble = str[n] - '0';
        } else if (str[n] >= 'a' && str[n] <= 'f') {
            nybble = (str[n] - 'a') + 0xA;
        } else {
            return 0;
        }

        value |= u64(nybble) << ((bitCount - 4) - n * 4);
    }

    return value;
}

static bool IsZeroBlock(const void* block, u32 size)
{
    const u8* blockU8 = reinterpret_cast<const u8*>(block);

    for (u32 i = 0; i < size; i++) {
        if (blockU8[i] != 0) {
            return false;
        }
    }

    return true;
}

static void SendAuthTokenSignature(
    GameSpy::GPConnection* connection, const char* authToken,
    GameSpy::GPIBuffer* outputBuffer, s32 esFd
)
{
    struct IOSCECCCert {
        u32 signatureType;
        u8 signature[0x3C];
        u8 signaturePad[0x40];

        char issuer[0x40];
        u32 publicKeyType;
        char name[0x40];
        u32 timestamp;

        u8 publicKey[0x3C];
        u8 publicKeyPad[0x3C];
    };

    static_assert(sizeof(IOSCECCCert) == 0x180);

    struct __attribute__((packed)) WWFCAuthSignature {
        /* 0x000:0x004 */ u32 deviceId;
        /* 0x004:0x008 */ u32 deviceTimestamp;
        /* 0x008:0x00C */ u32 caId;
        /* 0x00C:0x010 */ u32 msId;
        /* 0x010:0x018 */ u64 appTitleId;
        /* 0x018:0x054 */ u8 msSignature[0x3C];
        /* 0x054:0x090 */ u8 devicePublicKey[0x3C];
        /* 0x090:0x0CC */ u8 deviceSignature[0x3C];
        /* 0x0CC:0x108 */ u8 appPublicKey[0x3C];
        /* 0x108:0x144 */ u8 appSignature[0x3C];
        /* 0x144:0x148 */ u32 appTimestamp;
    };

    static_assert(sizeof(WWFCAuthSignature) == 0x148);

    static constexpr u32 ES_IOCTL_GET_DEVICE_CERT = 0x1E;
    static constexpr u32 ES_IOCTL_SIGN = 0x30;

    IOSCECCCert eccCert alignas(32) = {};
    RVL::IOVector vec[3 + 1] alignas(32) = {};

    vec[0].data = &eccCert;
    vec[0].size = sizeof(IOSCECCCert);
    s32 ret = RVL::IOS_Ioctlv(esFd, ES_IOCTL_GET_DEVICE_CERT, 0, 1, vec);
    if (ret != 0) {
        LOG_ERROR_FMT("Failed to get device certificate: %d", ret);
        return;
    }

    WWFCAuthSignature authSig = {};
    authSig.deviceId = DecodeUintString(eccCert.name + 2, 32);
    authSig.deviceTimestamp = eccCert.timestamp;

    if (authSig.deviceId == 0 || eccCert.signatureType != 0x00010002 ||
        eccCert.publicKeyType != 2 ||
        !IsZeroBlock(eccCert.signaturePad, sizeof(eccCert.signaturePad)) ||
        !IsZeroBlock(eccCert.publicKeyPad, sizeof(eccCert.publicKeyPad)) ||
        std::memcmp(eccCert.name, "NG", 2) ||
        !IsZeroBlock(eccCert.name + 0xA, 0x40 - 0xA)) {
        LOG_ERROR("Invalid device certificate");
        return;
    }

    authSig.caId = DecodeUintString(eccCert.issuer + 7, 32);
    authSig.msId = DecodeUintString(eccCert.issuer + 18, 32);

    if (authSig.caId == 0 || authSig.msId == 0 ||
        std::memcmp(eccCert.issuer, "Root-CA", 7) ||
        std::memcmp(eccCert.issuer + 0xF, "-MS", 3)) {
        LOG_ERROR("Invalid device certificate issuer");
        return;
    }

    std::memcpy(authSig.msSignature, eccCert.signature, 0x3C);
    std::memcpy(authSig.devicePublicKey, eccCert.publicKey, 0x3C);

    // Setup app issuer for sanity checking later
    char appIssuer[0x40];
    std::memcpy(appIssuer, eccCert.issuer, 0x1A);
    appIssuer[0x1A] = '-';
    std::memcpy(appIssuer + 0x1B, eccCert.name, 0xA);

    // Now call ES sign
    u8 eccSignature[0x3C + 0x4] alignas(32) = {};
    char authTokenAligned[GP_AUTHTOKEN_LEN] alignas(64) = {};

    s32 authTokenSize = std::strlen(authToken);
    if (authTokenSize > GP_AUTHTOKEN_LEN) {
        authTokenSize = GP_AUTHTOKEN_LEN;
    }
    std::memcpy(authTokenAligned, authToken, authTokenSize);

    eccCert = {};

    vec[0].data = &authTokenAligned;
    vec[0].size = authTokenSize;
    vec[1].data = eccSignature;
    vec[1].size = 0x3C;
    vec[2].data = &eccCert;
    vec[2].size = sizeof(IOSCECCCert);

    ret = RVL::IOS_Ioctlv(esFd, ES_IOCTL_SIGN, 1, 2, vec);
    if (ret != 0) {
        LOG_ERROR_FMT("Failed to sign auth token: %d", ret);
        return;
    }

    authSig.appTitleId = DecodeUintString(eccCert.name + 2, 64);
    authSig.appTimestamp = eccCert.timestamp;

    if (authSig.appTitleId == 0 ||
        !IsZeroBlock(eccCert.signaturePad, sizeof(eccCert.signaturePad)) ||
        !IsZeroBlock(eccCert.publicKeyPad, sizeof(eccCert.publicKeyPad)) ||
        std::memcmp(eccCert.name, "AP", 2) ||
        !IsZeroBlock(eccCert.name + 0x12, 0x40 - 0x12)) {
        LOG_ERROR("Invalid app certificate");
        return;
    }

    if (std::memcmp(eccCert.issuer, appIssuer, 0x25) ||
        !IsZeroBlock(eccCert.issuer + 0x25, 0x40 - 0x25)) {
        LOG_ERROR("Invalid app certificate issuer");
        return;
    }

    std::memcpy(authSig.deviceSignature, eccCert.signature, 0x3C);
    std::memcpy(authSig.appPublicKey, eccCert.publicKey, 0x3C);
    std::memcpy(authSig.appSignature, eccSignature, 0x3C);

    // Done signing, now base64 encode the result
    char b64AuthSig[0x400];
    s32 b64Len = DWC::DWC_Base64Encode(
        &authSig, sizeof(authSig), b64AuthSig, sizeof(b64AuthSig)
    );
    if (b64Len == -1 || b64Len == sizeof(b64AuthSig)) {
        LOG_ERROR("Could not fit the base64-encoded signature into the "
                  "provided buffer!");
        return;
    }

    // Add null terminator
    b64AuthSig[b64Len] = '\0';

    GameSpy::gpiAppendStringToBuffer(connection, outputBuffer, "\\wl:sig\\");
    GameSpy::gpiAppendStringToBuffer(connection, outputBuffer, b64AuthSig);
}

#if RMC

// Always check if the SAKE server has the latest friend info, as it can get
// outdated when switching between servers (such as Wiimmfi)
WWFC_DEFINE_PATCH = {Patch::WriteASM(
    WWFC_PATCH_LEVEL_SUPPORT | WWFC_PATCH_LEVEL_BUGFIX, //
    RMCXD_PORT(0x80672FCC, 0x8066B868, 0x80672638, 0x80661324), //
    1, ASM_LAMBDA(b 0x70)
)};

#endif

#if RMCN

// Same patch as above but for the Mario Kart Channel
WWFC_DEFINE_PATCH = {Patch::WriteASM(
    WWFC_PATCH_LEVEL_SUPPORT | WWFC_PATCH_LEVEL_BUGFIX, //
    RMCXN_PORT(0x801FD08C, 0x801FCFEC, 0x801FCE24, 0x801FD87C), //
    1, ASM_LAMBDA(b 0x6C)
)};

#endif

#if RMC

// Remove the needless wait during the login friend process. It doesn't seem to
// make a difference now due to the fast way we handle friend authorization.
// TODO: This could apply to other games as well
WWFC_DEFINE_PATCH = {Patch::WriteASM(
    WWFC_PATCH_LEVEL_SUPPORT, //
    RMCXD_PORT(0x800CE710, 0x800CE670, 0x800CE630, 0x800CE770), //
    1, ASM_LAMBDA(nop)
)};

#endif

} // namespace wwfc::Login

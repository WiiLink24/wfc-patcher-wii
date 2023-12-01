#include "import/dwc.h"
#include "import/gamespy.h"
#include "import/revolution.h"
#include "wwfcLog.hpp"
#include "wwfcPatch.hpp"
#include "wwfcPayload.hpp"
#include "wwfcUtil.h"
#include <cstring>

namespace wwfc::Login
{

int SendExtendedLogin(
    void* connection, GPIConnectData* data, void* outputBuffer
) asm("SendExtendedLogin");

static void SendAuthTokenSignature(
    void* connection, GPIConnectData* data, void* outputBuffer, s32 esFd
);

WWFC_DEFINE_PATCH = {Patch::CallWithCTR(
    WWFC_PATCH_LEVEL_CRITICAL, //
    ADDRESS_PATCH_GPISENDLOGIN, // 0x801007B0
    ASM_LAMBDA(
        // clang-format off
        mr      r3, r28;
        mr      r4, r29;
        addi    r5, r30, 0x210;
        b       SendExtendedLogin;
        // clang-format on
    )
)};

int SendExtendedLogin(
    void* connection, GPIConnectData* data, void* outputBuffer
)
{
    gpiAppendStringToBuffer(connection, outputBuffer, "\\payload_ver\\");
    gpiAppendIntToBuffer(
        connection, outputBuffer, Payload::Header.info.version
    );

    if (data->authtoken[0] != '\0') {
        s32 fd = IOS_Open("/dev/es", 0);
        if (fd >= 0) {
            SendAuthTokenSignature(connection, data, outputBuffer, fd);
            IOS_Close(fd);
        } else {
            LOG_ERROR_FMT("Failed to open ES: %d", fd);
        }
    }

    gpiAppendStringToBuffer(connection, outputBuffer, "\\final\\");

    return 0;
}

static u64 DecodeUintString(const char* str, u32 bitCount)
{
    u64 value = 0;

    for (int i = 0; i < (bitCount >> 2); i++) {
        u8 nybble = 0;
        if (str[i] >= '0' && str[i] <= '9') {
            nybble = str[i] - '0';
        } else if (str[i] >= 'a' && str[i] <= 'f') {
            nybble = (str[i] - 'a') + 0xA;
        } else {
            return 0;
        }

        value |= u64(nybble) << ((bitCount - 4) - i * 4);
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
    void* connection, GPIConnectData* data, void* outputBuffer, s32 esFd
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
    };

    static_assert(sizeof(WWFCAuthSignature) == 0x144);

    static constexpr u32 ES_IOCTL_GET_DEVICE_CERT = 0x1E;
    static constexpr u32 ES_IOCTL_SIGN = 0x30;

    IOSCECCCert eccCert alignas(32) = {};
    IOVector vec[3 + 1] alignas(32) = {};

    vec[0].data = &eccCert;
    vec[0].size = sizeof(IOSCECCCert);
    s32 ret = IOS_Ioctlv(esFd, ES_IOCTL_GET_DEVICE_CERT, 0, 1, vec);
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
        std::memcmp(eccCert.issuer + 0xF, "-MS", 3) ||
        !IsZeroBlock(eccCert.issuer + 0x1A, 0x40 - 0x1A)) {
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
    char authToken[GP_AUTHTOKEN_LEN] alignas(64) = {};

    s32 authTokenSize = std::strlen(data->authtoken);
    if (authTokenSize > GP_AUTHTOKEN_LEN) {
        authTokenSize = GP_AUTHTOKEN_LEN;
    }
    std::memcpy(authToken, data->authtoken, authTokenSize);

    vec[0].data = &authToken;
    vec[0].size = authTokenSize;
    vec[1].data = eccSignature;
    vec[1].size = 0x3C;
    vec[2].data = &eccCert;
    vec[2].size = sizeof(IOSCECCCert);

    ret = IOS_Ioctlv(esFd, ES_IOCTL_SIGN, 1, 2, vec);
    if (ret != 0) {
        LOG_ERROR_FMT("Failed to sign auth token: %d", ret);
        return;
    }

    authSig.appTitleId = DecodeUintString(eccCert.name + 2, 64);

    if (authSig.appTitleId == 0 || eccCert.timestamp != 0 ||
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
    s32 b64Len = DWC_Base64Encode(
        &authSig, sizeof(authSig), b64AuthSig, sizeof(b64AuthSig)
    );
    if (b64Len < 0 || b64Len >= 0x400) {
        LOG_ERROR("Could not base64 encode the signature");
        return;
    }

    // Add null terminator
    b64AuthSig[b64Len] = '\0';

    gpiAppendStringToBuffer(connection, outputBuffer, "\\wwfc_sig\\");
    gpiAppendStringToBuffer(connection, outputBuffer, b64AuthSig);
}

} // namespace wwfc::Login

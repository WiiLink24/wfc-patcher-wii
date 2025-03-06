#pragma once

#include <wwfcUtil.h>

#ifdef __cplusplus
extern "C" {

namespace GameSpy
{
#endif

typedef void* GPConnection;

typedef enum {
    GPNoError = 0,
} GPResult;

typedef enum {
    // Success
    GT2Success = 0,
    // Ran out of memory
    GT2OutOfMemory = 1,
    // Attempt rejected
    GT2Rejected = 2,
    // Networking error (could be local or remote)
    GT2NetworkError = 3,
    // Invalid or unreachable address
    GT2AddressError = 4,
    // A connection was attempted to an address that already has a connection on
    // the socket
    GT2DuplicateAddress = 5,
    // Time out reached
    GT2TimedOut = 6,
    // There was an error negotiating with the remote side
    GT2NegotiationError = 7,
    // The connection didn't exist
    GT2InvalidConnection = 8,
    // Used for VDP reliable messages containing voice data, no voice data in
    // reliable messages
    GT2InvalidMessage = 9,
    // The send failed
    GT2SendFailed = 10,
} GT2Result;

#define GP_AUTHTOKEN_LEN 256
#define GP_PARTNERCHALLENGE_LEN 256
#define GP_CDKEY_LEN 65

typedef enum {
    GPIFalse,
    GPITrue,
} GPIBool;

typedef enum GPEnum {
    GP_FATAL = 1,
    GP_NON_FATAL = 0,
} GPEnum;

#ifdef __cplusplus
static_assert(sizeof(GPIBool) == 0x4);
#endif

typedef struct {
    char* buffer;
    int size;
    int length;
    int position;
} GPIBuffer;

#ifdef __cplusplus
static_assert(sizeof(GPIBuffer) == 0x10);
#endif

typedef struct {
    char serverChallenge[128];
    char userChallenge[33];
    char passwordHash[33];
    char authtoken[GP_AUTHTOKEN_LEN];
    char partnerchallenge[GP_PARTNERCHALLENGE_LEN];
    char cdkey[GP_CDKEY_LEN];
    GPIBool newuser;
} GPIConnectData;

#ifdef __cplusplus
static_assert(sizeof(GPIConnectData) == 0x308);
#endif

typedef struct {
    FILL(0x000, 0x210);
    GPIBuffer outputBuffer;
    FILL(0x220, 0x5E0);
    GPIBuffer updateproBuffer;
} GPIConnection;

LONGCALL GPResult gpiAppendStringToBuffer( //
    GPConnection* connection, GPIBuffer* outputBuffer, const char* buffer
) AT(ADDRESS_gpiAppendStringToBuffer);

LONGCALL int gpiAppendIntToBuffer( //
    GPConnection* connection, GPIBuffer* outputBuffer, int num
) AT(ADDRESS_gpiAppendIntToBuffer);

LONGCALL GPIBool gpiValueForKey( //
    const char* command, const char* key, char* value, int length
) AT(ADDRESS_gpiValueForKey);

#if RMC

GPResult
gpiSendLocalInfo(GPConnection* gpConnection, const char* key, const char* value)
{
    GameSpy::GPIConnection* gpiConnection =
        reinterpret_cast<GameSpy::GPIConnection*>(*gpConnection);

    {
        GPResult gpResult = gpiAppendStringToBuffer(
            gpConnection, &gpiConnection->updateproBuffer, key
        );
        if (gpResult != GPNoError) {
            return gpResult;
        }
    }

    return gpiAppendStringToBuffer(
        gpConnection, &gpiConnection->updateproBuffer, value
    );
}

#endif

LONGCALL void gpiCallErrorCallback( //
    GPConnection* connection, GPResult result, GPEnum fatal
) AT(ADDRESS_gpiCallErrorCallback);

LONGCALL GT2Result gt2CreateSocket( //
    void* sock, const char* localAddress, int outgoingBufferSize,
    int incomingBufferSize, void* callback
) AT(ADDRESS_gt2CreateSocket);

LONGCALL const char* gt2AddressToString( //
    u32 ip, u16 port, char string[22]
) AT(ADDRESS_gt2AddressToString);

#ifdef __cplusplus
}
}
#endif

#pragma once

#include <wwfcUtil.h>

#ifdef __cplusplus
extern "C" {

namespace GameSpy
{
#endif

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

#ifdef __cplusplus
static_assert(sizeof(GPIBool) == 0x4);
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
    u32 outputBuffer;
} GPIConnection;

LONGCALL int gpiAppendStringToBuffer( //
    void* connection, void* outputBuffer, const char* buffer
) AT(ADDRESS_gpiAppendStringToBuffer);

LONGCALL int gpiAppendIntToBuffer( //
    void* connection, void* outputBuffer, int num
) AT(ADDRESS_gpiAppendIntToBuffer);

typedef void* GPConnection;

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

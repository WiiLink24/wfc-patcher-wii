#pragma once

#include <wwfcUtil.h>

#ifdef __cplusplus
extern "C" {

namespace GameSpy
{
#endif

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

LONGCALL int gpiAppendStringToBuffer( //
    void* connection, void* outputBuffer, const char* buffer
) AT(ADDRESS_gpiAppendStringToBuffer);

LONGCALL int gpiAppendIntToBuffer( //
    void* connection, void* outputBuffer, int num
) AT(ADDRESS_gpiAppendIntToBuffer);

#ifdef __cplusplus
}
}
#endif

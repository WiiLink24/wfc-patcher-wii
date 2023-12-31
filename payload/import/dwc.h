#pragma once

#include <wwfcUtil.h>

#ifdef __cplusplus
extern "C" {

namespace DWC
{
#endif

typedef struct {
    FILL(0x00, 0x1C);
    u32 profileId;
} DWCUserData;

typedef struct {
    /* 0x000 / 0x802F1CB8 */ s32 error;
    /* 0x004 / 0x802F1CBC */ char authToken[0x12D];
    /* 0x131 / 0x802F1DE9 */ char challenge[0x4A];
    /* 0x17B / 0x802F1E33 */ char locator[0x35];
    /* 0x1B0 / 0x802F1E68 */ u64 userId;
    /* 0x1B8 / 0x802F1E70 */ s32 profStatus;
    /* 0x1BC / 0x802F1E74 */ u32 pad_0x1BC;
    /* 0x1C0 / 0x802F1E78 */ u64 dateTime;
    /* 0x1C8 / 0x802F1E80 */ s32 nhttpResult;
    /* 0x1CC / 0x802F1E74 */ u32 unk_0x1CC;
} DWCAuthResult;

static_assert(sizeof(DWCAuthResult) == 0x1D0);

typedef struct {
    /* 0x000 / 0x802F1E88 */ u32 statusData;
    /* 0x004 / 0x802F1E8C */ char serviceHost[0x41];
    /* 0x045 / 0x802F1ECD */ char serviceToken[0x12F];
} DWCSvcLocResult;

static_assert(sizeof(DWCSvcLocResult) == 0x174);

typedef struct {
    /* 0x000 / 0x802F2000 */ char prWords[0x34];
    /* 0x034 / 0x802F2034 */ s32 result;
} DWCProfResult;

static_assert(sizeof(DWCProfResult) == 0x38);

extern DWCAuthResult s_auth_result AT(ADDRESS_AUTH_RESULT);
extern DWCSvcLocResult s_svl_result AT(ADDRESS_AUTH_RESULT + 0x1D0);
extern DWCProfResult s_prof_result AT(ADDRESS_AUTH_RESULT + 0x1D0 + 0x178);

// 0x803862D0
extern void* stpFriendCnt AT(ADDRESS_stpFriendCnt);

typedef struct {
    /* 0x0 */ u32 pad_0x0;
    /* 0x4 */ u32 state;
} DWCLoginContext;

// 0x803862E8
extern DWCLoginContext* stpLoginCnt AT(ADDRESS_stpLoginCnt);

LONGCALL DWCUserData* DWCi_GetUserData( //
    void
) AT(ADDRESS_DWCi_GetUserData);

LONGCALL s32 DWC_Base64Encode( //
    const void* in, u32 inSize, char* out, u32 outMaxSize
) AT(ADDRESS_DWC_Base64Encode);

#ifdef __cplusplus
}
}
#endif

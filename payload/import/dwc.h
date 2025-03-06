#pragma once

#include "gamespy.h"

#ifdef __cplusplus
extern "C" {

namespace DWC
{
#endif

typedef struct {
    /* 0x00 */ FILL(0x00, 0x1C);
    /* 0x1C */ int profileId;
    /* 0x20 */ FILL(0x20, 0x24);
    /* 0x24 */ u32 gameCode;
    /* 0x28 */ FILL(0x28, 0x40);
} DWCUserData;

#ifdef __cplusplus
static_assert(sizeof(DWCUserData) == 0x40);
#endif

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

#ifdef __cplusplus
static_assert(sizeof(DWCAuthResult) == 0x1D0);
#endif

typedef struct {
    /* 0x000 / 0x802F1E88 */ u32 statusData;
    /* 0x004 / 0x802F1E8C */ char serviceHost[0x41];
    /* 0x045 / 0x802F1ECD */ char serviceToken[0x12F];
} DWCSvcLocResult;

#ifdef __cplusplus
static_assert(sizeof(DWCSvcLocResult) == 0x174);
#endif

typedef struct {
    /* 0x000 / 0x802F2000 */ char prWords[0x34];
    /* 0x034 / 0x802F2034 */ s32 result;
} DWCProfResult;

#ifdef __cplusplus
static_assert(sizeof(DWCProfResult) == 0x38);
#endif

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

#if RMC

LONGCALL int DWC_CloseConnectionHard(u8 playerAid)
    AT(RMCXD_PORT(0x800D2000, 0x800D1F60, 0x800D1F20, 0x800D2060));

typedef struct {
    /* 0x00 */ u32 profileId;
    /* 0x04 */ u8 _04[0x30 - 0x04];
} DWCiNodeInfo;

#  ifdef __cplusplus
static_assert(sizeof(DWCiNodeInfo) == 0x30);
#  endif

LONGCALL DWCiNodeInfo* DWCi_NodeInfoList_GetNodeInfoForAid(u8 playerAid)
    AT(RMCXD_PORT(0x800E7EE0, 0x800E7E40, 0x800E7E00, 0x800E7F40));

LONGCALL int DWC_CheckFriendKey(const DWCUserData* userData, u64 friendKey)
    AT(RMCXD_PORT(0x800EB8D8, 0x800EB838, 0x800EB7F8, 0x800EB950));

typedef struct {
    /* 0x0 */ GameSpy::GPConnection* connection;
} DWCMatchContext;

extern DWCMatchContext*
    stpMatchCnt AT(RMCXD_PORT(0x8038630C, 0x80381F8C, 0x80385C8C, 0x8037432C));

#endif

LONGCALL DWCUserData* DWCi_GetUserData( //
    void
) AT(ADDRESS_DWCi_GetUserData);

LONGCALL s32 DWC_Base64Encode( //
    const void* in, u32 inSize, char* out, u32 outMaxSize
) AT(ADDRESS_DWC_Base64Encode);

#if RMC

LONGCALL s32 DWC_Base64Decode(
    const void* source, u32 sourceSize, char* destination, u32 destinationSize
) AT(RMCXD_PORT(0x800CC974, 0x800CC8D4, 0x800CC894, 0x800CC9D4));

#endif

LONGCALL s32 DWCi_SetError( //
    s32 errorClass, s32 errorCode
) AT(ADDRESS_DWCi_SetError);

#ifdef __cplusplus
}
}
#endif

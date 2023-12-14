#pragma once

#include <wwfcUtil.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    FILL(0x00, 0x1C);
    u32 profileId;
} DWCUserData;

LONGCALL DWCUserData* DWCi_GetUserData( //
    void
) AT(ADDRESS_DWCi_GetUserData);

LONGCALL s32 DWC_Base64Encode( //
    const void* in, u32 inSize, char* out, u32 outMaxSize
) AT(ADDRESS_DWC_Base64Encode);

#ifdef __cplusplus
}
#endif

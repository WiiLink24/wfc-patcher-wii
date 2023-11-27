#pragma once

#include <wwfcUtil.h>

#ifdef __cplusplus
extern "C" {
#endif

LONGCALL s32 DWC_Base64Encode( //
    const void* in, u32 inSize, char* out, u32 outMaxSize
) AT(ADDRESS_DWC_Base64Encode);

#ifdef __cplusplus
}
#endif

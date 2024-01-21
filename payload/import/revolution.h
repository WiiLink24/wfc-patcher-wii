#pragma once

#include <wwfcUtil.h>

#ifdef __cplusplus
extern "C" {

namespace RVL
{
#endif

LONGCALL void OSReport( //
    const char* format, ...
) AT(ADDRESS_OSReport);

LONGCALL void DCFlushRange( //
    void* ptr, u32 size
) AT(ADDRESS_DCFlushRange);

LONGCALL void ICInvalidateRange( //
    void* ptr, u32 size
) AT(ADDRESS_ICInvalidateRange);

typedef struct {
    void* data;
    u32 size;
} IOVector;

LONGCALL s32 IOS_Open( //
    const char* path, u32 flags
) AT(ADDRESS_IOS_Open);

LONGCALL s32 IOS_Close( //
    s32 fd
) AT(ADDRESS_IOS_Close);

LONGCALL s32 IOS_Ioctlv( //
    s32 fd, u32 cmd, u32 in_count, u32 out_count, IOVector* vec
) AT(ADDRESS_IOS_Ioctlv);

LONGCALL bool SCGetProductSN( //
    u32* serial
) AT(ADDRESS_SCGetProductSN);

#ifdef __cplusplus
}
}
#endif

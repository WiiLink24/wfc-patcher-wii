#pragma once

#include <wwfcUtil.h>

#ifdef __cplusplus
extern "C" {
#endif

LONGCALL void OSReport(const char* format, ...) AT(ADDRESS_OSReport);

LONGCALL void DCFlushRange(void* ptr, u32 size) //
    AT(ADDRESS_DCFlushRange);
LONGCALL void ICInvalidateRange(void* ptr, u32 size) //
    AT(ADDRESS_ICInvalidateRange);

#ifdef __cplusplus
}
#endif

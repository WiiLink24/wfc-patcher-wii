#pragma once

#include <cstddef>
#include <wwfcUtil.h>

extern "C" {

#if RMC

LONGCALL int snprintf(char* buffer, size_t bufferSize, const char* format, ...)
    AT(RMCXD_PORT(0x80011938, 0x80010DD8, 0x8001185C, 0x800119A0));

#endif
}
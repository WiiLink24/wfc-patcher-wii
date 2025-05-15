#pragma once

#include <wwfcLibC.hpp>
#include <wwfcCommon.h>

namespace wwfc::GPReport
{

#if RMC

void Report(const char* key, const char* string);
void ReportU32(const char* key, u32 uint);
void ReportB64Encode(const char* key, const void* data, std::size_t dataSize);

#endif

} // namespace wwfc::GPReport

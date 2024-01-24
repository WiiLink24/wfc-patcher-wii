#pragma once

#include "import/mkwNetUserHandler.hpp"

namespace wwfc::GPReport
{

#if RMC

void Report(const char* key, const char* string);
void ReportB64Encode(const char* key, const void* data, size_t dataSize);

#endif

} // namespace wwfc::GPReport
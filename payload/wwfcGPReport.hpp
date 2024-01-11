#pragma once

#include "import/mkwNet.hpp"

namespace wwfc::GPReport
{

#if RMC

void ReportUSER(mkw::Net::USERPacket* packet);

#endif

} // namespace wwfc::GPReport
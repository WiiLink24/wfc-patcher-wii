#pragma once

#include "import/mkwNet.hpp"

namespace wwfc::GPReport
{

#if RMC

void ReportUSER(mkw::Net::USERHandler::Packet* packet);

#endif

} // namespace wwfc::GPReport
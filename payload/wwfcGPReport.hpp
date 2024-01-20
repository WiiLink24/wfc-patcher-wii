#pragma once

#include "import/mkwNetUserHandler.hpp"

namespace wwfc::GPReport
{

#if RMC

void ReportUser(mkw::Net::UserHandler::Packet* packet);

#endif

} // namespace wwfc::GPReport
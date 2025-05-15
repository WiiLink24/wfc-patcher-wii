#pragma once

#if RMC

#  include "import/mkw/net/net.hpp"

namespace wwfc::mkw::Security
{

bool IsRacePacketValid(
    const Net::RacePacket* racePacket, u32 racePacketSize, u8 playerAid
);

} // namespace wwfc::mkw::Security

#endif // RMC
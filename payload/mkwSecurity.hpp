#pragma once

#if RMC

#  include "import/mkw/net/NetManager.hpp"

namespace wwfc::Security
{

bool IsRacePacketValid(
    const mkw::NetRacePacket* racePacket, u32 racePacketSize, u8 playerAid
);

} // namespace wwfc::Security

#endif // RMC
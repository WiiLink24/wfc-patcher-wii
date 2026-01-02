#pragma once

#if RMC

#  include "timer.hpp"

namespace wwfc::mkw::System
{

class RaceManager
{
public:
    class Player
    {
    public:
        FILL(0x00, 0x40);
        /* 0x40 */ Timer::Time* m_raceFinishTime;
    };
};

} // namespace wwfc::mkw::System

#endif // RMC
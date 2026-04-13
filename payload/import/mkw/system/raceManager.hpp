#pragma once

#if RMC

#  include "timer.hpp"

namespace wwfc::mkw::System
{

class RaceManager
{
public:
    static RaceManager* Instance()
    {
        extern RaceManager* s_instance AT(RMCXD_PORT(
            0x809BD730, 0x809B8F70, 0x809BC790, 0x809ABD70, 0x809BDFB0
        ));
        return s_instance;
    }

    class Player
    {
    public:
        FILL(0x00, 0x08);
        /* 0x08 */ u8 m_id;
        FILL(0x09, 0x40);
        /* 0x40 */ Time* m_raceFinishTime;
    };

    enum class State {
        Intro = 0,
        Countdown = 1,
        Race = 2,
        Finished = 3,
        FinishedTimeAttack = 4,
    };

    FILL(0x00, 0x14);

    /* 0x14 */ Timer* m_timer;

    FILL(0x14, 0x28);

    /* 0x28 */ State m_state;
};

} // namespace wwfc::mkw::System

#endif // RMC
#pragma once

#include <wwfcUtil.h>

namespace mkw::System
{

#if RMC

class RaceManager
{
public:
    u32 timer() const
    {
        return m_timer;
    }

    static RaceManager* Instance()
    {
        return s_instance;
    }

private:
    /* 0x00 */ u8 _00[0x20 - 0x00];
    /* 0x20 */ u32 m_timer;
    /* 0x24 */ u8 _24[0x4C - 0x24];

    static RaceManager* s_instance
        AT(RMCXD_PORT(0x809BD730, 0x809B8F70, 0x809BC790, 0x809ABD70));
};

static_assert(sizeof(RaceManager) == 0x4C);

#endif

} // namespace mkw::System

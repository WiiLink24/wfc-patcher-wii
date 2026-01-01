#pragma once

#if RMC

namespace wwfc::mkw::System
{

class Timer
{
public:
    struct Time {
        /* 0x0 */ void* m_vtable;
        /* 0x4 */ u16 m_minutes;
        /* 0x6 */ u8 m_seconds;
        /* 0x8 */ u16 m_milliseconds;
        /* 0xA */ bool m_valid;
    };
};

} // namespace wwfc::mkw::System

#endif // RMC
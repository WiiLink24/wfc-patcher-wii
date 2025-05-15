#pragma once

#if RMC

#  include <wwfcUtil.h>

namespace wwfc::mkw::HostSystem
{

class SystemManager
{
public:
    enum class MatchingArea {
        Japan = 0,
        UnitedStates = 1,
        Europe = 2,
        Australia = 3,
        Taiwan = 4,
        Korea = 5,
        China = 6,
    };

    MatchingArea matchingArea()
    {
        return m_matchingArea;
    }

    static SystemManager* Instance()
    {
        return s_instance;
    }

private:
    /* 0x0000 */ u8 _0000[0x0084 - 0x0000];
    /* 0x0084 */ MatchingArea m_matchingArea;
    /* 0x0088 */ u8 _0088[0x1100 - 0x0088];

    static SystemManager* s_instance
        AT(RMCXD_PORT(0x80386000, 0x80381C80, 0x80385980, 0x80374020));
};

static_assert(sizeof(SystemManager) == 0x1100);

} // namespace wwfc::mkw::HostSystem

#endif // RMC
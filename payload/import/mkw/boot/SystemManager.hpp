#pragma once

#if RMC

namespace wwfc::mkw
{

class SystemManager
{
public:
    enum class EWifiRegion {
        JP = 0, // Japan
        US = 1, // The Americas
        EU = 2, // Europe
        AU = 3, // Australia
        TW = 4, // Taiwan
        KR = 5, // South Korea
        CH = 6, // China; not CN for some reason
    };

    EWifiRegion getWifiRegion()
    {
        return m_wifiRegion;
    }

    static SystemManager* Instance()
    {
        return s_instance;
    }

private:
    /* 0x0000 */ u8          _0000[0x0084 - 0x0000];
    /* 0x0084 */ EWifiRegion m_wifiRegion;
    /* 0x0088 */ u8          _0088[0x1100 - 0x0088];

    static SystemManager*
        s_instance AT(RMCXD_PORT(0x80386000, 0x80381C80, 0x80385980, 0x80374020, 0x80385680));
};

static_assert(sizeof(SystemManager) == 0x1100);

} // namespace wwfc::mkw

#endif // RMC
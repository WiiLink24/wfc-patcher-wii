#pragma once

#if RMC

#  include "section.hpp"
#  include <wwfcUtil.h>

namespace wwfc::mkw::UI
{

class SectionManager
{
public:
    Section* currentSection()
    {
        return m_currentSection;
    }

    static SectionManager* Instance()
    {
        return s_instance;
    }

private:
    /* 0x00 */ Section* m_currentSection;
    /* 0x04 */ u8 _04[0x9C - 0x04];

    static SectionManager* s_instance
        AT(RMCXD_PORT(0x809C1E38, 0x809BD508, 0x809C0E98, 0x809B0478));
};

static_assert(sizeof(SectionManager) == 0x9C);

} // namespace wwfc::mkw::UI

#endif // RMC
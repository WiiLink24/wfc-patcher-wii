#pragma once

#include <wwfcUtil.h>

namespace mkw::Util
{

#if RMC

class Random
{
public:
    Random* dt(Random* random, s32 type)
    {
        LONGCALL Random* dt(Random * random, s32 type)
            AT(RMCXD_PORT(0x80555538, 0x8054F518, 0x80554EB8, 0x80543590));

        return dt(random, type);
    }

private:
    u8 _00[0x18 - 0x00];
};

static_assert(sizeof(Random) == 0x18);

#endif

} // namespace mkw::Util

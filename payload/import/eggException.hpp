#pragma once

#include <wwfcUtil.h>

namespace EGG
{

class Exception
{
public:
    virtual ~Exception();

    static void SetUserCallBack(void* arg)
    {
#if RMC
        LONGCALL void SetUserCallBack(void* arg)
            AT(RMCXD_PORT(0x802267F0, 0x8022646C, 0x80226710, 0x80226B64));

        SetUserCallBack(arg);
#endif
    }
};

static_assert(sizeof(Exception) == 0x04);

} // namespace EGG

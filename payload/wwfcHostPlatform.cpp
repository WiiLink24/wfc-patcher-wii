#include "wwfcHostPlatform.hpp"
#include "import/revolution.h"

namespace wwfc::HostPlatform
{

static bool IsDolphinImpl()
{
    // Newer versions of Dolphin have an IOS device called /dev/dolphin
    s32 fd = RVL::IOS_Open("/dev/dolphin", 0);
    if (fd >= 0) {
        RVL::IOS_Close(fd);
        return true;
    }

    // Older versions do not have this device, but they can be detected by the
    // lack of /dev/sha
    fd = RVL::IOS_Open("/dev/sha", 0);
    if (fd >= 0) {
        RVL::IOS_Close(fd);
        return false;
    }

    return fd == -6; // ENOENT
}

bool IsDolphin()
{
    static bool isDolphin = IsDolphinImpl();
    return isDolphin;
}

} // namespace wwfc::HostPlatform
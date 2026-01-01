#include "wwfcHostPlatform.hpp"
#include "import/revolution.h"

namespace wwfc::HostPlatform
{

const s32 g_dolphinFd = RVL::IOS_Open("/dev/dolphin", 0);
static const bool s_isDolphin = g_dolphinFd >= 0 || []() {
    // Older versions do not have the /dev/dolphin device, but they can be
    // detected by the lack of /dev/sha
    s32 fd = RVL::IOS_Open("/dev/sha", 0);
    if (fd >= 0) {
        RVL::IOS_Close(fd);
    }
    return fd == -6; // IOS_ERROR_NOEXISTS
}();

bool IsDolphin()
{
    return s_isDolphin;
}

} // namespace wwfc::HostPlatform
#pragma once

#if RMC

#  include <wwfcTypes.h>

namespace wwfc::mkw::UI
{

struct FormatParam {
    /* 0x00 */ s32 numbers[9];
    /* 0x24 */ u32 messageIds[9];
    /* 0x48 */ const void* miis[9];
    /* 0x6C */ u8 licenseIds[9];
    /* 0x78 */ u32 playerIds[9];
    /* 0x9C */ const wchar_t* strings[9];
    /* 0xC0 */ u8 _C0[0xC4 - 0xC0];
};

static_assert(sizeof(FormatParam) == 0xC4);

} // namespace wwfc::mkw::UI

#endif // RMC
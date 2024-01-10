#pragma once

#include <wwfcCommon.h>
#include <wwfcUtil.h>

namespace mkw::UI
{

struct FormatParam {
    /* 0x00 */ s32 numbers[9];
    /* 0x24 */ s32 msgIds[9];
    /* 0x48 */ void* miis[9];
    /* 0x6C */ u8 licenses[9];
    /* 0x78 */ s32 playerIds[9];
    /* 0x9C */ const u16* strings[9];
    /* 0xC0 */ u32 unk_0xC0;
};

} // namespace mkw::UI
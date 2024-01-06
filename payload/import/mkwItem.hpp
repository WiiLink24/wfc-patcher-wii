#pragma once

#include <wwfcUtil.h>

// https://github.com/SeekyCt/mkw-structures/blob/master/itembehaviour.h
namespace mkw::Item
{

struct ItemBehaviourEntry {
    /* 0x00 */ u8 _00[0x18 - 0x00];
    /* 0x18 */ void (*useFunction)(void* kartItem);
};

static_assert(sizeof(ItemBehaviourEntry) == 0x1C);

extern ItemBehaviourEntry itemBehaviourTable[0x13] AT(
    RMCXD_PORT(0x809C36A0, 0x809BEE98, 0x809C2700, 0x809B1CE0)
);

} // namespace mkw::Item

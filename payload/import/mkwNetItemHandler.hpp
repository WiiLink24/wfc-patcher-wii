#pragma once

#include <wwfcUtil.h>

namespace mkw::Net
{

#if RMC

// https://github.com/SeekyCt/mkw-structures/blob/master/itemhandler.h
class ItemHandler
{
public:
    struct Packet {
        /* 0x00 */ u8 _00;
        /* 0x01 */ u8 heldItem;
        /* 0x02 */ u8 trailedItem;
        /* 0x03 */ u8 _03[0x08 - 0x03];
    };

    static_assert(sizeof(Packet) == 0x08);

    static ItemHandler* Instance()
    {
        return s_instance;
    }

private:
    /* 0x000 */ u8 _000[0x184 - 0x000];

    static ItemHandler* s_instance
        AT(RMCXD_PORT(0x809C20F8, 0x809BD950, 0x809C1158, 0x809B0738));
};

static_assert(sizeof(ItemHandler) == 0x184);

#endif

} // namespace mkw::Net

#pragma once

#include <wwfcCommon.h>
#include <wwfcMii.hpp>

namespace mkw::Net
{

struct RACEPacket {
    enum EType {
        HEADER,
        RACEHEADER_1,
        RACEHEADER_2,
        ROOM_SELECT,
        RACEDATA,
        USER,
        ITEM,
        EVENT,
    };

    /* 0x00 */ u32 pad;
    /* 0x04 */ u32 checksum;
    /* 0x08 */ u8 sizes[8];
};

static_assert(sizeof(RACEPacket) == 0x10);

struct USERPacket {
    /* 0x00 */ u32 miiGroupBitflags;
    /* 0x04 */ u16 miiGroupCount;
    /* 0x06 */ u16 _0x06;
    /* 0x08 */ wwfc::Mii::MiiData miiData[2];
    /* 0xA0 */ u64 wiiFriendCode;
    /* 0xA8 */ u64 friendCode;
    /* 0xB0 */ u8 country;
    /* 0xB1 */ u8 state;
    /* 0xB2 */ u16 city;
    /* 0xB4 */ u16 longitude;
    /* 0xB6 */ u16 latitude;
    /* 0xB8 */ u16 vr;
    /* 0xBA */ u16 br;
    /* 0xBC */ char regionChar;
    /* 0xBD */ u8 regionId;
    /* 0xBE */ u16 _0xBE;
};

static_assert(sizeof(USERPacket) == 0xC0);

} // namespace mkw::Net

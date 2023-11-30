#pragma once

#include <wwfcCommon.h>
#include <wwfcUtil.h>

namespace wwfc::Mii
{

struct RFLCreateID {
    u32 miiID;
    u32 consoleID;
};

static_assert(sizeof(RFLCreateID) == 0x8);

constexpr u32 RFL_NAME_LEN = 10;
constexpr u32 RFL_CREATOR_LEN = 10;

struct PACKED RFLiCharData {
    // 0x00
    u16 padding0 : 1;
    u16 sex : 1;
    u16 birthMonth : 4;
    u16 birthDay : 5;
    u16 favoriteColor : 4;
    u16 favorite : 1;

    u16 name[RFL_NAME_LEN]; // 0x02
    u8 height; // 0x16
    u8 build; // 0x17
    RFLCreateID createID; // 0x18

    // 0x20
    u16 faceType : 3;
    u16 faceColor : 3;
    u16 faceTex : 4;
    u16 padding2 : 3;
    u16 localonly : 1;
    u16 type : 2;

    // 0x22
    u16 hairType : 7;
    u16 hairColor : 3;
    u16 hairFlip : 1;
    u16 padding3 : 5;

    // 0x24
    u16 eyebrowType : 5;
    u16 eyebrowRotate : 5;
    u16 padding4 : 6;

    // 0x26
    u16 eyebrowColor : 3;
    u16 eyebrowScale : 4;
    u16 eyebrowY : 5;
    u16 eyebrowX : 4;

    // 0x28
    u16 eyeType : 6;
    u16 eyeRotate : 5;
    u16 eyeY : 5;

    // 0x2A
    u16 eyeColor : 3;
    u16 eyeScale : 4;
    u16 eyeX : 4;
    u16 padding5 : 5;

    // 0x2C
    u16 noseType : 4;
    u16 noseScale : 4;
    u16 noseY : 5;
    u16 padding6 : 3;

    // 0x2E
    u16 mouthType : 5;
    u16 mouthColor : 2;
    u16 mouthScale : 4;
    u16 mouthY : 5;

    // 0x30
    u16 glassType : 4;
    u16 glassColor : 3;
    u16 glassScale : 4;
    u16 glassY : 5;

    // 0x32
    u16 mustacheType : 2;
    u16 beardType : 2;
    u16 beardColor : 3;
    u16 beardScale : 4;
    u16 beardY : 5;

    // 0x34
    u16 moleType : 1;
    u16 moleScale : 4;
    u16 moleY : 5;
    u16 moleX : 5;
    u16 padding8 : 1;

    u16 creatorName[RFL_CREATOR_LEN]; // at 0x36
};

static_assert(sizeof(RFLiCharData) == 0x4A);

struct RFLiStoreData {
    /* 0x00 */ RFLiCharData data;
    /* 0x4A */ u16 checksum;
};

static_assert(sizeof(RFLiStoreData) == 0x4C);

} // namespace wwfc::Mii

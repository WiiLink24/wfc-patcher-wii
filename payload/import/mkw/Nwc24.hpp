#pragma once

#if RMC || RMCN

#  include "import/rfl.h"

namespace wwfc::mkw
{

// RACE: SubmitScores->ScoreDatas->ScoreData->playerinfobase64
struct Nwc24PlayerInfo {
    /* 0x00 */ RFL::RFLiStoreData mii;
    /* 0x4C */ u8 controller;
    /* 0x4D */ u8 character;
    /* 0x4E */ u8 vehicle;
    /* 0x4F */ u8 country;
};

static_assert(sizeof(Nwc24PlayerInfo) == 0x50);

// SAKE: mariokartwii/FriendInfo->info
struct Nwc24FriendInfo {
    /* 0x00 */ RFL::RFLiStoreData mii;
    /* 0x4C */ u32 wiiNumberHi;
    /* 0x50 */ u32 wiiNumberLo;
    /* 0x54 */ u32 gameCode; // RMC?
    /* 0x58 */ u32 location;
    /* 0x5C */ u16 latitude;
    /* 0x5E */ u16 longitude;
};

static_assert(sizeof(Nwc24FriendInfo) == 0x60);

} // namespace wwfc::mkw

#endif // RMC || RMCN

#pragma once

#include <wwfcMii.hpp>

namespace mkw::Net
{

#if RMC

class UserHandler
{
public:
    struct Packet {
        bool isMiiGroupCountValid() const
        {
            return miiGroupCount == maxMiis;
        }

        bool isVersusRatingValid() const
        {
            return vr >= minRating && vr <= maxRating;
        }

        bool isBattleRatingValid() const
        {
            return br >= minRating && br <= maxRating;
        }

        /* 0x00 */ u32 miiGroupBitflags;
        /* 0x04 */ u16 miiGroupCount;
        /* 0x06 */ u16 _0x06;
        /* 0x08 */ wwfc::Mii::RFLiStoreData miiData[2];
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

    private:
        static const u16 maxMiis = 2;
        static const u16 minRating = 1;
        static const u16 maxRating = 9999;
    };

    static_assert(sizeof(Packet) == 0xC0);

    static UserHandler* Instance()
    {
        return s_instance;
    }

private:
    /* 0x000 */ u8 _000[0x9F0 - 0x000];

    static UserHandler* s_instance
        AT(RMCXD_PORT(0x809C2108, 0x809BD958, 0x809C1168, 0x809B0748));
};

static_assert(sizeof(UserHandler) == 0x9F0);

#endif

} // namespace mkw::Net

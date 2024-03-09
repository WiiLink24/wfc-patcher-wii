#pragma once

#include "import/dwc.h"
#include <wwfcMii.hpp>

namespace mkw::Net
{

#if RMC

class UserHandler
{
public:
    struct __attribute__((packed)) Packet {
        int miiCount() const
        {
            return std::countr_one(miiGroupBitFlags);
        }

        bool isMiiGroupBitFlagsValid() const
        {
            int numMiis = miiCount();

            return numMiis >= 1 && numMiis <= 2;
        }

        bool isMiiGroupCountValid() const
        {
            return miiGroupCount == s_maxMiis;
        }

        bool isFriendCodeValid() const
        {
            DWC::DWCUserData userData{};
            userData.gameCode = 0x524D434A; // RMCJ

            return DWC::DWC_CheckFriendKey(&userData, friendCode);
        }

        bool isVersusRatingValid() const
        {
            return vr >= s_minRating && vr <= s_maxRating;
        }

        bool isBattleRatingValid() const
        {
            return br >= s_minRating && br <= s_maxRating;
        }

        bool isValid() const
        {
            if (!isMiiGroupBitFlagsValid()) {
                return false;
            }

            if (!isMiiGroupCountValid()) {
                return false;
            }

            if (!isFriendCodeValid()) {
                return false;
            }

            if (!isVersusRatingValid()) {
                return false;
            }

            if (!isBattleRatingValid()) {
                return false;
            }

            return true;
        }

        /* 0x00 */ u32 miiGroupBitFlags;
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
        static const u16 s_maxMiis = 2;
        static const u16 s_minRating = 1;
        static const u16 s_maxRating = 9999;
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

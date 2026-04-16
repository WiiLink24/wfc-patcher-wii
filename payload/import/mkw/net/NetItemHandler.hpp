#pragma once

#if RMC

#  include "import/mkw/item/ItemType.hpp"
#  include "wwfcEnum.hpp"

namespace wwfc::mkw
{

// https://github.com/SeekyCt/mkw-structures/blob/master/itemhandler.h
class NetItemHandler
{
public:
    struct [[gnu::packed]] Packet {
        enum class EHeldPhase : u8 {
            NO_ITEM    = 0,
            DECIDED    = 1,
            FINALIZED  = 2,
            RESET      = 3,
            THREE_LEFT = 4,
            TWO_LEFT   = 5,
            ONE_LEFT   = 6,
            REJECTED   = 7,
        };

        enum class ETrailPhase : u8 {
            NO_ITEM         = 0,
            THREE_LEFT_ODD  = 1,
            TWO_LEFT_ODD    = 2,
            ONE_LEFT_ODD    = 3,
            USED            = 4,
            THREE_LEFT_EVEN = 5,
            TWO_LEFT_EVEN   = 6,
            ONE_LEFT_EVEN   = 7,
        };

        constexpr bool isHeldPhaseValid() const
        {
            switch (heldPhase) {
            case EHeldPhase::NO_ITEM:
            case EHeldPhase::REJECTED:
                return heldItem == EItemType::EMPTY;

            case EHeldPhase::DECIDED:
                return ItemDefaults::isValid(heldItem);

            case EHeldPhase::FINALIZED:
                if (heldItem == EItemType::EMPTY) {
                    // The item was rejected by another client
                    return true;
                }
                return ItemDefaults::isValid(heldItem);

            case EHeldPhase::THREE_LEFT:
            case EHeldPhase::TWO_LEFT:
                return ItemDefaults::hasQuantity(heldItem);

            case EHeldPhase::ONE_LEFT:
                return ItemDefaults::canHold(heldItem);

            // This value is not transmitted over the network
            case EHeldPhase::RESET:
            default:
                return false;
            }
        }

        constexpr bool isTrailPhaseValid() const
        {
            switch (trailPhase) {
            case ETrailPhase::NO_ITEM:
            case ETrailPhase::USED:
                return trailedItem == EItemType::EMPTY;

            case ETrailPhase::THREE_LEFT_ODD:
            case ETrailPhase::TWO_LEFT_ODD:
            case ETrailPhase::THREE_LEFT_EVEN:
            case ETrailPhase::TWO_LEFT_EVEN:
                return ItemDefaults::hasQuantity(trailedItem);

            case ETrailPhase::ONE_LEFT_ODD:
            case ETrailPhase::ONE_LEFT_EVEN:
                return ItemDefaults::canTrail(trailedItem);

            default:
                return false;
            }
        }

        /* 0x00 */ u8                  _00;
        /* 0x01 */ Enum<u8, EItemType> heldItem;
        /* 0x02 */ Enum<u8, EItemType> trailedItem;
        /* 0x03 */ EHeldPhase          heldPhase;
        /* 0x04 */ ETrailPhase         trailPhase;
        /* 0x05 */ u8                  _05[0x08 - 0x05];
    };

    static_assert(sizeof(Packet) == 0x08);

    static NetItemHandler* Instance()
    {
        return s_instance;
    }

private:
    /* 0x000 */ u8 _000[0x184 - 0x000];

    static NetItemHandler*
        s_instance AT(RMCXD_PORT(0x809C20F8, 0x809BD950, 0x809C1158, 0x809B0738, 0x809C2990));
};

static_assert(sizeof(NetItemHandler) == 0x184);

} // namespace wwfc::mkw

#endif // RMC
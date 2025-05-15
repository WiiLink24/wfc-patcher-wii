#if RMC

#  pragma once

#  include "import/mkw/item.hpp"
#  include "wwfcUtil.h"

namespace wwfc::mkw::Net
{

// https://github.com/SeekyCt/mkw-structures/blob/master/itemhandler.h
class ItemHandler
{
public:
    struct __attribute__((packed)) Packet {
        enum class HeldPhase : u8 {
            NoItem = 0,
            Decided = 1,
            Finalised = 2,
            Reset = 3,
            ThreeLeft = 4,
            TwoLeft = 5,
            OneLeft = 6,
            Rejected = 7,
        };

        enum class TrailPhase : u8 {
            NoItem = 0,
            ThreeLeftOdd = 1,
            TwoLeftOdd = 2,
            OneLeftOdd = 3,
            Used = 4,
            ThreeLeftEven = 5,
            TwoLeftEven = 6,
            OneLeftEven = 7,
        };

        bool isHeldPhaseValid() const
        {
            using namespace mkw::Item;

            ItemBox item = static_cast<ItemBox>(heldItem);

            switch (heldPhase) {
            case HeldPhase::NoItem:
            case HeldPhase::Rejected: {
                return item == ItemBox::NoItem;
            }
            case HeldPhase::Decided: {
                return IsItemValid(item);
            }
            case HeldPhase::Finalised: {
                if (item == ItemBox::NoItem) {
                    // The item was rejected by another client
                    return true;
                }

                return IsItemValid(item);
            }
            case HeldPhase::ThreeLeft:
            case HeldPhase::TwoLeft: {
                return DoesItemHaveQuantity(item);
            }
            case HeldPhase::OneLeft: {
                return CanHoldItem(item);
            }
            // This value is not transmitted over the network
            case HeldPhase::Reset:
            default: {
                return false;
            }
            }
        }

        bool isTrailPhaseValid() const
        {
            using namespace mkw::Item;

            ItemBox item = static_cast<ItemBox>(trailedItem);

            switch (trailPhase) {
            case TrailPhase::NoItem:
            case TrailPhase::Used: {
                return item == ItemBox::NoItem;
            }
            case TrailPhase::ThreeLeftOdd:
            case TrailPhase::TwoLeftOdd:
            case TrailPhase::ThreeLeftEven:
            case TrailPhase::TwoLeftEven: {
                return DoesItemHaveQuantity(item);
            }
            case TrailPhase::OneLeftOdd: {
            case TrailPhase::OneLeftEven:
                return CanTrailItem(item);
            }
            default: {
                return false;
            }
            }
        }

        /* 0x00 */ u8 _00;
        /* 0x01 */ u8 heldItem;
        /* 0x02 */ u8 trailedItem;
        /* 0x03 */ HeldPhase heldPhase;
        /* 0x04 */ TrailPhase trailPhase;
        /* 0x05 */ u8 _05[0x08 - 0x05];
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

} // namespace wwfc::mkw::Net

#endif // RMC
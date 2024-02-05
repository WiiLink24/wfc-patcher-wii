#pragma once

#include "mkwItem.hpp"
#include <cstddef>

namespace mkw::Net
{

#if RMC

// https://github.com/SeekyCt/mkw-structures/blob/master/eventhandler.h
class EventHandler
{
public:
    struct Packet {
        struct EventInfo {
            enum class EventType : u8 {
                NoEvent = 0,
                ItemUsed = 1,
                ItemThrown = 2,
                ItemObjectHit = 3,
                ItemLockedOn = 5,
                ItemDropped = 7,
            };

            bool isItemObjectValid() const
            {
                using namespace mkw::Item;

                return IsItemObjectValid(static_cast<ItemObject>(itemObject));
            }

            bool isValid() const
            {
                using namespace mkw::Item;

                if (!isItemObjectValid()) {
                    return false;
                }

                ItemObject item = static_cast<ItemObject>(itemObject);

                switch (eventType) {
                case EventType::NoEvent: {
                    return item == ItemObject::NoObject;
                }
                case EventType::ItemUsed: {
                    return CanUseItem(ItemObjectToItemBox(item));
                }
                case EventType::ItemThrown: {
                    return CanThrowItem(ItemObjectToItemBox(item));
                }
                case EventType::ItemObjectHit: {
                    return CanHitItemObject(item);
                }
                case EventType::ItemLockedOn: {
                    return CanItemObjectLockOn(item);
                }
                case EventType::ItemDropped: {
                    return CanDropItemObject(item);
                }
                default: {
                    return true;
                }
                }
            }

            u8 getEventDataSize() const
            {
                return GetEventDataSize(itemObject, eventType);
            }

            /* 0x00 */ EventType eventType : 3;
            /* 0x00 */ u8 itemObject : 5;
        };

        static_assert(sizeof(EventInfo) == 0x01);

        bool containsInvalidItemObject() const
        {
            for (size_t n = 0; n < sizeof(eventInfo); n++) {
                if (!eventInfo[n].isItemObjectValid()) {
                    return true;
                }
            }

            return false;
        }

        bool isValid(u8 packetSize) const
        {
            u32 expectedPacketSize = sizeof(eventInfo);

            for (size_t n = 0; n < sizeof(eventInfo); n++) {
                if (!eventInfo[n].isValid()) {
                    return false;
                }

                expectedPacketSize += eventInfo[n].getEventDataSize();
            }

            return expectedPacketSize == packetSize;
        }

        /* 0x00 */ EventInfo eventInfo[0x18];
        /* 0x18 */ u8 _18[0xF8 - 0x18];
    };

    static_assert(sizeof(Packet) == 0xF8);

    static u8
    GetEventDataSize(u8 itemObject, Packet::EventInfo::EventType eventType)
    {
        LONGCALL u8 GetEventDataSize(
            u8 itemObject, Packet::EventInfo::EventType eventType
        ) AT(RMCXD_PORT(0x8079D76C, 0x80794760, 0x8079CDD8, 0x8078BB2C));

        return GetEventDataSize(itemObject, eventType);
    }

    static EventHandler* Instance()
    {
        return s_instance;
    }

private:
    /* 0x0000 */ u8 _0000[0x2B88 - 0x0000];

    static EventHandler* s_instance
        AT(RMCXD_PORT(0x809C20F0, 0x809BD928, 0x809C1150, 0x809B0730));
};

static_assert(sizeof(EventHandler) == 0x2B88);

#endif

} // namespace mkw::Net

#pragma once

#if RMC

#  include "import/mkw/item.hpp"
#  include "wwfcLibC.hpp"
#  include "wwfcUtil.h"

namespace wwfc::mkw::Net
{

// https://github.com/SeekyCt/mkw-structures/blob/master/eventhandler.h
class EventHandler
{
public:
    struct __attribute__((packed)) Packet {
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

                ItemObject item = static_cast<ItemObject>(itemObject);

                if (item == ItemObject::NoObject) {
                    return true;
                }

                return IsItemObjectValid(item);
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

        struct ItemUsedEvent {
            bool isValid(u8 itemObject, u8 playerAid) const
            {
                using namespace mkw::Item;

                ItemObject item = static_cast<ItemObject>(itemObject);

                switch (item) {
                case ItemObject::ThunderCloud: {
                    return this->playerAid == playerAid;
                }
                default: {
                    return true;
                }
                }
            }

            /* 0x00 */ u8 _00[0x02 - 0x00];
            /* 0x02 */ u8 playerAid;
        };

        static_assert(sizeof(ItemUsedEvent) == 0x03);

        bool containsInvalidItemObject() const
        {
            for (std::size_t n = 0; n < sizeof(eventInfo); n++) {
                if (!eventInfo[n].isItemObjectValid()) {
                    return true;
                }
            }

            return false;
        }

        bool isValid(u8 packetSize, u8 playerAid) const
        {
            std::size_t expectedPacketSize = sizeof(eventInfo);

            for (std::size_t n = 0; n < sizeof(eventInfo); n++) {
                if (!eventInfo[n].isValid()) {
                    return false;
                }

                expectedPacketSize += eventInfo[n].getEventDataSize();
            }

            if (expectedPacketSize != packetSize) {
                return false;
            }

            expectedPacketSize = 0;
            for (std::size_t n = 0; n < sizeof(eventInfo); n++) {
                EventInfo info = eventInfo[n];
                u8 itemObject = info.itemObject;
                const u8* data = eventData + expectedPacketSize;

                switch (info.eventType) {
                case EventInfo::EventType::ItemUsed: {
                    const ItemUsedEvent* itemUsedEvent =
                        reinterpret_cast<const ItemUsedEvent*>(data);

                    if (!itemUsedEvent->isValid(itemObject, playerAid)) {
                        return false;
                    }
                }
                default: {
                }
                }

                expectedPacketSize += info.getEventDataSize();
            }

            return true;
        }

        /* 0x00 */ EventInfo eventInfo[0x18];
        /* 0x18 */ u8 eventData[0xF8 - 0x18];
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

} // namespace wwfc::mkw::Net

#endif // RMC
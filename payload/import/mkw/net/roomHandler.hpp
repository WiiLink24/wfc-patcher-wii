#pragma once

#if RMC

namespace wwfc::mkw
{

// https://github.com/SeekyCt/mkw-structures/blob/master/roomhandler.h
class NetRoomHandler
{
public:
    struct __attribute__((packed)) Packet {
        enum class Event : u8 {
            StartRoom = 1,
            RegisterFriend = 2,
            JoinMessage = 3,
            ChatMessage = 4,
        };

        /* 0x00 */ Event event;
        /* 0x01 */ u8 _01[0x04 - 0x01];
    };

    static_assert(sizeof(Packet) == 0x04);

    static NetRoomHandler* Instance()
    {
        return s_instance;
    }

private:
    /* 0x00 */ u8 _00[0x80 - 0x00];

    static NetRoomHandler* s_instance AT(
        RMCXD_PORT(0x809C20E0, 0x809BD920, 0x809C1140, 0x809B0720, 0x809C2978)
    );
};

static_assert(sizeof(NetRoomHandler) == 0x80);

} // namespace wwfc::mkw

#endif // RMC
#pragma once

#if RMC

#  include <wwfcUtil.h>

namespace wwfc::mkw::Net
{

class MatchHeaderHandler
{
public:
    struct __attribute__((packed)) Packet {
        enum class Vehicle : u8 {
            None = 0xFF,
        };

        enum class Character : u8 {
            None = 0xFF,
        };

        enum class Course : u8 {
            None = 0xFF,
        };

        struct Player {
            /* 0x00 */ Vehicle vehicle;
            /* 0x01 */ Character character;
        };

        static_assert(sizeof(Player) == 0x02);

        /* 0x00 */ u8 _00[0x0E - 0x00];
        /* 0x0E */ Player player[2];
        /* 0x12 */ u8 _12[0x16 - 0x12];
        /* 0x16 */ Course course;
        /* 0x17 */ u8 _17[0x28 - 0x17];
    };

    static_assert(sizeof(Packet) == 0x28);

    static MatchHeaderHandler* Instance()
    {
        return s_instance;
    }

private:
    /* 0x000 */ u8 _000[0x260 - 0x000];

    static MatchHeaderHandler* s_instance
        AT(RMCXD_PORT(0x809C2118, 0x809BD940, 0x809C1178, 0x809B0758));
};

static_assert(sizeof(MatchHeaderHandler) == 0x260);

} // namespace wwfc::mkw::Net

#endif // RMC
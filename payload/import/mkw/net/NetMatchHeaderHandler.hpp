#pragma once

#if RMC

#  include "../Registry.hpp"
#  include "wwfcEnum.hpp"

namespace wwfc::mkw
{

class NetMatchHeaderHandler
{
public:
    struct [[gnu::packed]] Packet {
        static constexpr u8 NONE = 0xFF;

        struct [[gnu::packed]] Player {
            /* 0x00 */ Enum<u8, EVehicle>   vehicle;
            /* 0x01 */ Enum<u8, ECharacter> character;
        };

        static_assert(sizeof(Player) == 0x02);

        /* 0x00 */ u8                _00[0x0E - 0x00];
        /* 0x0E */ Player            player[2];
        /* 0x12 */ u8                _12[0x16 - 0x12];
        /* 0x16 */ Enum<u8, ECourse> course;
        /* 0x17 */ u8                _17[0x28 - 0x17];
    };

    static_assert(sizeof(Packet) == 0x28);

    static NetMatchHeaderHandler* Instance()
    {
        return s_instance;
    }

private:
    /* 0x000 */ u8 _000[0x260 - 0x000];

    static NetMatchHeaderHandler*
        s_instance AT(RMCXD_PORT(0x809C2118, 0x809BD940, 0x809C1178, 0x809B0758, 0x809C29B0));
};

static_assert(sizeof(NetMatchHeaderHandler) == 0x260);

} // namespace wwfc::mkw

#endif // RMC
#pragma once

#include <wwfcUtil.h>

namespace mkw::Net
{

#if RMC

// https://github.com/SeekyCt/mkw-structures/blob/master/selecthandler.h
class SelectHandler
{
public:
    struct Packet {
        enum class Character : u8 {
            NotSelected = 0x30,
        };

        enum class Vehicle : u8 {
            NotSelected = 0x24,
        };

        enum class CourseVote : u8 {
            NotSelected = 0x43,
            Random = 0xFF,
        };

        enum class SelectedCourse : u8 {
            NotSelected = 0xFF,
        };

        enum class EngineClass : u8 {
            e100cc = 1,
            e150cc = 2,
            eMirrorMode = 3,
        };

        struct Player {
            /* 0x00 */ u8 _00[0x04 - 0x00];
            /* 0x04 */ Character character;
            /* 0x05 */ Vehicle vehicle;
            /* 0x06 */ CourseVote courseVote;
            /* 0x07 */ u8 _07;
        };

        static_assert(sizeof(Player) == 0x08);

        /* 0x00 */ u8 _00[0x10 - 0x00];
        /* 0x10 */ Player player[2];
        /* 0x20 */ u8 _20[0x34 - 0x20];
        /* 0x34 */ SelectedCourse selectedCourse;
        /* 0x35 */ u8 _35[0x37 - 0x35];
        /* 0x37 */ EngineClass engineClass;
    };

    static_assert(sizeof(Packet) == 0x38);

    void decideEngineClass()
    {
        LONGCALL void decideEngineClass(SelectHandler * selectHandler)
            AT(RMCXD_PORT(0x80661A5C, 0x80659B20, 0x806610C8, 0x8064FD74));

        decideEngineClass(this);
    }

    Packet& sendPacket()
    {
        return m_sendPacket;
    }

    static SelectHandler* Instance()
    {
        return s_instance;
    }

private:
    /* 0x000 */ u8 _000[0x008 - 0x000];
    /* 0x008 */ Packet m_sendPacket;
    /* 0x040 */ u8 _040[0x3F8 - 0x040];

    static SelectHandler* s_instance
        AT(RMCXD_PORT(0x809C2100, 0x809BD930, 0x809C1160, 0x809B0740));
};

static_assert(sizeof(SelectHandler) == 0x3F8);

#endif

} // namespace mkw::Net

#pragma once

#if RMC

#  include "import/egg/vector.hpp"

namespace wwfc::mkw::System
{

// https://github.com/riidefi/mkw/blob/master/source/game/system/CourseMap.hpp#L359-L373
class MapdataCannonPoint
{
public:
    enum class CannonType : s16 {
        Direct,
        Curved,
        CurvedSlow,

        Default = -1,
    };

    struct Data {
        /* 0x00 */ EGG::Vector3f position;
        /* 0x0C */ EGG::Vector3f rotation;
        /* 0x18 */ u16 id;
        /* 0x1A */ CannonType cannonType;
    };

    static_assert(sizeof(Data) == 0x1C);

    MapdataCannonPoint(const Data* data)
      : m_data(data)
    {
    }

private:
    /* 0x00 */ const Data* m_data;
};

static_assert(sizeof(MapdataCannonPoint) == 0x04);

// https://github.com/riidefi/mkw/blob/master/source/game/system/CourseMap.hpp#L543-L557
class MapdataItemPoint
{
public:
    struct Data {
        /* 0x00 */ EGG::Vector3f position;
        /* 0x0C */ f32 deviation;
        /* 0x10 */ u16 parameters[2];
    };

    static_assert(sizeof(Data) == 0x14);

    MapdataItemPoint(const Data* data)
      : m_data(data)
    {
    }

private:
    /* 0x00 */ const Data* m_data;
    /* 0x04 */ u8 _04[0x14 - 0x04];
};

static_assert(sizeof(MapdataItemPoint) == 0x14);

} // namespace wwfc::mkw::System

#endif // RMC

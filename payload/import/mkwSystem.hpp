#pragma once

#include "eggVector.hpp"

namespace mkw::System
{

#if RMC

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
        EGG::Vector3f position;
        EGG::Vector3f rotation;
        u16 id;
        CannonType cannonType;
    };

    MapdataCannonPoint(const Data* data)
      : m_data(data)
    {
    }

private:
    const Data* m_data;
};

static_assert(sizeof(MapdataCannonPoint) == 0x4);

// https://github.com/riidefi/mkw/blob/master/source/game/system/CourseMap.hpp#L543-L557
class MapdataItemPoint
{
public:
    struct Data {
        EGG::Vector3f position;
        f32 deviation;
        u16 parameters[2];
    };

    MapdataItemPoint(const Data* data)
      : m_data(data)
    {
    }

private:
    const Data* m_data;
    u8 _04[0x14 - 0x04];
};

static_assert(sizeof(MapdataItemPoint) == 0x14);

#endif

} // namespace mkw::System

#pragma once

#include <wwfcCommon.h>

namespace EGG
{

struct Vector3f {
    Vector3f(f32 x, f32 y, f32 z)
      : x(x)
      , y(y)
      , z(z)
    {
    }

    f32 x;
    f32 y;
    f32 z;
};

} // namespace EGG

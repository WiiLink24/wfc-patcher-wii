#pragma once

#include <wwfcCommon.h>

namespace wwfc::EGG
{

struct Vector3f {
    Vector3f(f32 x, f32 y, f32 z)
      : x(x)
      , y(y)
      , z(z)
    {
    }

    /* 0x00 */ f32 x;
    /* 0x04 */ f32 y;
    /* 0x08 */ f32 z;
};

static_assert(sizeof(Vector3f) == 0x0C);

} // namespace wwfc::EGG

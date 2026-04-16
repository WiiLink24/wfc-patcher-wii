#pragma once

#include <wwfcTypes.h>

namespace wwfc::EGG
{

struct Vector3f {
    Vector3f(float x, float y, float z)
      : x(x)
      , y(y)
      , z(z)
    {
    }

    /* 0x00 */ float x;
    /* 0x04 */ float y;
    /* 0x08 */ float z;
};

static_assert(sizeof(Vector3f) == 0x0C);

} // namespace wwfc::EGG

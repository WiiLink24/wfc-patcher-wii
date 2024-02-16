#pragma once

#include <ogc/gx.h>

class Assets
{
public:
    static GXTexObj GetTexture(const char* path, f32* width, f32* height);
};
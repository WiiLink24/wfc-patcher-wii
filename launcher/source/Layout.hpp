#pragma once

#include <gctypes.h>

class Layout
{
public:
    Layout() = default;

    virtual void Init();
    virtual void Calc();
    virtual void Draw();

    float m_x;
    float m_y;
    float m_z;

    float m_width;
    float m_height;

    bool m_visible;
    u8 m_alpha;
};
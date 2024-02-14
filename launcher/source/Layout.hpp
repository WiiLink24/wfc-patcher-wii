#pragma once

#include <gctypes.h>

class Layout
{
public:
    enum class Anchor {
        TOP_LEFT,
        TOP_CENTER,
        TOP_RIGHT,
        MIDDLE_LEFT,
        MIDDLE_CENTER,
        MIDDLE_RIGHT,
        BOTTOM_LEFT,
        BOTTOM_CENTER,
        BOTTOM_RIGHT,
    };

    Layout() = default;

    virtual void Calc() = 0;
    virtual void Draw() = 0;

    float m_x = 0.0f;
    float m_y = 0.0f;
    float m_z = 0.0f;

    float m_width = 100.0f;
    float m_height = 100.0f;

    bool m_visible = true;
    u8 m_alpha = 0xFF;

    Anchor m_anchor = Anchor::MIDDLE_CENTER;

protected:
    float GetAnchoredX() const
    {
        // TODO
        return m_x;
    }

    float GetAnchoredY() const
    {
        // TODO
        return m_y;
    }
};
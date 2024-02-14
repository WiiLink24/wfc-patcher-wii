#pragma once

#include "Layout.hpp"

class Layout_LoadingIcon : public Layout
{
public:
    void Init();
    void Calc() override;
    void Draw() override;

    void StopAnimation()
    {
        m_animate = false;
        m_alpha = 128;
    }

    void StartAnimation()
    {
        if (m_animate) {
            return;
        }

        m_frame = 21;
        m_animate = true;
        m_alpha = 245;
    }

private:
    u32 m_frame = 21;
    bool m_animate = false;
};
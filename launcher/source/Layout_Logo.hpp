#pragma once

#include "Layout.hpp"

class Layout_Logo : public Layout
{
public:
    void Init();
    void Calc() override;
    void Draw() override;

    void SetAnimState(bool in)
    {
        AnimState animState = in ? AnimState::IN : AnimState::OUT;
        if (m_animState == animState) {
            return;
        }

        m_animState = animState;
        m_animFrame = 0;
    }

private:
    struct VtxColorContext {
        u32 m_color = 0;
        u32 m_nextColor = 0;
        s32 m_time = -1;
        s32 m_end;

        u8 r;
        u8 g;
        u8 b;
    };

    VtxColorContext m_vtxColor[4];

    enum class AnimState {
        NONE,
        IN,
        OUT,
    };

    AnimState m_animState = AnimState::IN;
    s32 m_animFrame = 0;
};
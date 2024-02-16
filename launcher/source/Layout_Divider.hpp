#pragma once

#include "Layout.hpp"

class Layout_Divider : public Layout
{
public:
    void Init();
    void Calc() override;

    void Draw() override;
    void DrawBack();
    void DrawFront();

    static constexpr u32 ANIM_LINE_FRAMES = 16;

    void StartFadeIn()
    {
        if (m_animState == AnimState::IN) {
            return;
        }

        m_animState = AnimState::IN;
        m_animFrame = 0;
    }

    void StartFadeOutBack()
    {
        if (m_animState == AnimState::OUT_BACK) {
            return;
        }

        m_animState = AnimState::OUT_BACK;
        m_animFrame = 0;
    }

    void StartFadeOut()
    {
        if (m_animState == AnimState::OUT) {
            return;
        }

        m_animState = AnimState::OUT;
        m_animFrame = 0;
    }

    bool IsFadeDone() const
    {
        return m_animFrame >= ANIM_LINE_FRAMES;
    }

private:
    enum class AnimState {
        NONE,
        IN,
        OUT_BACK,
        OUT,
    };

    AnimState m_animState = AnimState::IN;
    u32 m_animFrame = 0;
};
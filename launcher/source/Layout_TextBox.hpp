#pragma once

#include "Layout.hpp"
#include <ogc/gx.h>
#include <ogc/system.h>
#include <wchar.h>

class Layout_TextBox : public Layout
{
public:
    void Init(const sys_fontheader* fontHeader);
    void Calc() override;
    void Draw() override;

    void SetText(const wchar_t* text)
    {
        m_text = text;
    }

    static constexpr u32 FADE_LENGTH = 30;

    void SetNextText(const wchar_t* text)
    {
        if (m_text == nullptr) {
            m_text = text;
            return;
        }

        if (wcscmp(m_text, text) == 0 ||
            (m_nextText != nullptr && wcscmp(m_nextText, text) == 0)) {
            return;
        }

        if (m_nextText != nullptr && m_fadeFrame > FADE_LENGTH / 2) {
            // Convert fade in to fade out
            m_fadeFrame = FADE_LENGTH - m_fadeFrame;
        }

        if (m_nextText == nullptr) {
            m_fadeFrame = 0;
        }

        m_nextText = text;
    }

    void SetFontSize(float fontSize)
    {
        m_fontSize = fontSize;
    }

    void SetFontColor(GXColor fontColor)
    {
        m_fontColor = fontColor;
    }

    void SetMonospace(bool monospace)
    {
        m_monospace = monospace;
    }

    void SetKerning(float kerning)
    {
        m_kerning = kerning;
    }

    void SetLeading(float leading)
    {
        m_leading = leading;
    }

    void SetTextAnchor(Anchor textAnchor)
    {
        m_textAnchor = textAnchor;
    }

private:
    const wchar_t* m_text;
    float m_fontSize;
    GXColor m_fontColor;
    const sys_fontheader* m_fontHeader;

    bool m_monospace = false;
    float m_kerning = 0;
    float m_leading = 6;

    Anchor m_textAnchor = Anchor::MIDDLE_CENTER;

    const wchar_t* m_nextText;
    u32 m_fadeFrame = 0;
};
#pragma once

#include "Layout.hpp"
#include <ogc/gx.h>
#include <ogc/system.h>

class Layout_TextBox : public Layout
{
public:
    void Init(const sys_fontheader* fontHeader);
    void Calc() override;
    void Draw() override;

    void SetText(const wchar_t* text);

private:
    const wchar_t* m_text;
    float m_fontSize;
    GXColor m_fontColor;

    const sys_fontheader* m_fontHeader;

    bool m_monospace = false;

    float m_kerning = 0;
    float m_leading = 6;

    Anchor m_textAnchor = Anchor::MIDDLE_CENTER;
};
#include "Layout_TextBox.hpp"
#include <cassert>
#include <cstdio>
#include <cstring>
#include <ogc/gx.h>
#include <ogc/system.h>
#include <wchar.h>

void Layout_TextBox::Init(const sys_fontheader* fontHeader)
{
    m_text = nullptr;
    m_fontSize = 1.5f;
    m_fontColor = (GXColor){0xFF, 0xFF, 0xFF, 0xFF};
    m_fontHeader = fontHeader;

    m_width = 400.0f;
    m_height = 400.0f;

    m_nextText = nullptr;
    m_fadeFrame = 0;
}

void Layout_TextBox::Calc()
{
    if (m_nextText != nullptr) {
        m_fadeFrame++;
        if (m_fadeFrame >= FADE_LENGTH) {
            m_text = m_nextText;
            m_nextText = nullptr;
            m_fadeFrame = 0;
            m_alpha = 0xFF;
        } else if (m_fadeFrame == FADE_LENGTH / 2) {
            m_text = m_nextText;
            m_alpha = 0;
        } else if (m_fadeFrame < FADE_LENGTH / 2) {
            m_alpha = 0xFF - (0xFF * m_fadeFrame) / (FADE_LENGTH / 2);
        } else {
            m_alpha =
                (0xFF * (m_fadeFrame - FADE_LENGTH / 2)) / (FADE_LENGTH / 2);
        }
    }
}

void Layout_TextBox::Draw()
{
    if (!m_visible || m_alpha == 0 || m_text == nullptr ||
        m_fontHeader == nullptr) {
        return;
    }

    float sheetWidth = float(m_fontHeader->sheet_width);
    float sheetHeight = float(m_fontHeader->sheet_height);
    float cellWidth = float(m_fontHeader->cell_width);
    float cellHeight = float(m_fontHeader->cell_height);
    float texHeight = cellHeight / sheetHeight;
    float texWidth = cellWidth / sheetWidth;
    float posWidth = cellWidth * m_fontSize;
    float posHeight = cellHeight * m_fontSize;

    // SYS_GetFontTexture output
    void* image;
    s32 cellX, cellY, width;

    // Calculate full text width and height for determining squish
    float fullTextWidth = 0, fullTextHeight = 0, currentLineWidth = 0;
    u32 len = wcslen(m_text);
    for (u32 i = 0; i < len; i++) {
        if (m_text[i] == L'\n') {
            currentLineWidth = 0;
            fullTextHeight += posHeight + m_leading;
            continue;
        }

        if (m_monospace) {
            currentLineWidth += posWidth + m_kerning;
            if (currentLineWidth > fullTextWidth) {
                fullTextWidth = currentLineWidth;
            }
        } else {
            SYS_GetFontTexture(m_text[i], &image, &cellX, &cellY, &width);
            assert(image != nullptr);
            currentLineWidth += float(width) * m_fontSize + m_kerning;
        }

        if (currentLineWidth > fullTextWidth) {
            fullTextWidth = currentLineWidth;
        }
    }

    fullTextHeight += posHeight;

    if (fullTextWidth == 0 || fullTextHeight == 0) {
        return;
    }

    float squishX = 1.0;
    float squishY = 1.0;

    if (fullTextWidth > m_width) {
        squishX = m_width / fullTextWidth;
        fullTextWidth = m_width;
    }

    if (fullTextHeight > m_height) {
        squishY = m_height / fullTextHeight;
        fullTextHeight = m_height;
    }

    posWidth *= squishX;
    posHeight *= squishY;

    float startX = GetAnchoredX(), startY = GetAnchoredY();

    // TODO: Anchor each line
    switch (m_textAnchor) {
    case Anchor::TOP_LEFT:
    case Anchor::MIDDLE_LEFT:
    case Anchor::BOTTOM_LEFT:
        startX -= m_width / 2;
        break;

    case Anchor::TOP_CENTER:
    case Anchor::MIDDLE_CENTER:
    case Anchor::BOTTOM_CENTER:
        startX -= fullTextWidth / 2;
        break;

    case Anchor::TOP_RIGHT:
    case Anchor::MIDDLE_RIGHT:
    case Anchor::BOTTOM_RIGHT:
        startX += m_width / 2 - fullTextWidth * squishX;
        break;
    }

    switch (m_textAnchor) {
    case Anchor::TOP_LEFT:
    case Anchor::TOP_CENTER:
    case Anchor::TOP_RIGHT:
        startY += m_height / 2;
        break;

    case Anchor::MIDDLE_LEFT:
    case Anchor::MIDDLE_CENTER:
    case Anchor::MIDDLE_RIGHT:
        startY += fullTextHeight / 2;
        break;

    case Anchor::BOTTOM_LEFT:
    case Anchor::BOTTOM_CENTER:
    case Anchor::BOTTOM_RIGHT:
        startY -= m_height / 2 + fullTextHeight * squishY;
        break;
    }

    // Set up for drawing f32 quads with tex map
    GX_ClearVtxDesc();
    GX_SetVtxDesc(GX_VA_POS, GX_DIRECT);
    GX_SetVtxDesc(GX_VA_TEX0, GX_DIRECT);
    GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
    GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);

    // Set material channels, TEV, texture
    GX_SetNumChans(1);
    GX_SetNumTexGens(1);
    GX_SetTevOp(GX_TEVSTAGE0, GX_REPLACE);
    GX_SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLORNULL);
    GX_SetTevColorIn(GX_TEVSTAGE0, GX_CC_C0, GX_CC_C1, GX_CC_TEXC, GX_CC_ZERO);
    GX_SetTevAlphaIn(GX_TEVSTAGE0, GX_CA_A0, GX_CA_A1, GX_CA_TEXA, GX_CA_ZERO);
    GX_SetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY);

    GX_SetTevColor(
        GX_TEVREG0, (GXColor){0x00, 0x00, 0x00, 0x00}
    ); // Black color
    GXColor fontColor = m_fontColor;
    fontColor.a = m_alpha;
    GX_SetTevColor(GX_TEVREG1, fontColor); // White color
    GX_SetTevColor(
        GX_TEVREG2, (GXColor){0x00, 0x00, 0x00, 0x00}
    ); // Alpha color

    GX_InvalidateTexAll();

    // Blending
    GX_SetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_SET);

    GX_SetColorUpdate(GX_TRUE);
    GX_SetAlphaUpdate(GX_TRUE);

    // Draw text
    float x = startX, y = startY - posHeight;
    for (u32 i = 0; i < len; i++) {
        if (m_text[i] == L'\n') {
            x = startX;
            y -= posHeight + m_leading;
            continue;
        }

        SYS_GetFontTexture(m_text[i], &image, &cellX, &cellY, &width);
        assert(image != nullptr);

        GXTexObj texObj;
        GX_InitTexObj(
            &texObj, image, m_fontHeader->sheet_width,
            m_fontHeader->sheet_height, GX_TF_I4, GX_CLAMP, GX_CLAMP, GX_FALSE
        );
        GX_LoadTexObj(&texObj, GX_TEXMAP0);

        float texX = float(cellX) / sheetWidth;
        float texY = float(cellY) / sheetHeight;
        if (!m_monospace) {
            texWidth = float(width) / sheetWidth;
            posWidth = float(width) * m_fontSize * squishX;
        }

        GX_Begin(GX_QUADS, GX_VTXFMT0, 4);
        {
            GX_Position3f32(x, y, -15.0); // top left
            GX_TexCoord2f32(texX, texY + texHeight);
            GX_Position3f32(x + posWidth, y, -15.0); // top right
            GX_TexCoord2f32(texX + texWidth, texY + texHeight);
            GX_Position3f32(x + posWidth, y + posHeight, -15.0); // bottom right
            GX_TexCoord2f32(texX + texWidth, texY);
            GX_Position3f32(x, y + posHeight, -15.0); // bottom left
            GX_TexCoord2f32(texX, texY);
        }
        GX_End();

        x += posWidth + m_kerning;
    }
}
#include "Layout_Logo.hpp"
#include "Assets.hpp"
#include <ogc/gx.h>

static GXTexObj s_texObj = {};

void Layout_Logo::Init()
{
    s_texObj = Assets::GetTexture("wfc-logo.tpl", &m_width, &m_height);
}

void Layout_Logo::Calc()
{
}

void Layout_Logo::Draw()
{
    if (!m_visible || m_alpha == 0 || m_width == 0 || m_height == 0) {
        return;
    }

    // Set up for drawing f32 quads with tex map
    GX_ClearVtxDesc();
    GX_SetVtxDesc(GX_VA_POS, GX_DIRECT);
    GX_SetVtxDesc(GX_VA_TEX0, GX_DIRECT);
    // GX_SetVtxDesc(GX_VA_CLR0, GX_DIRECT);
    GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
    GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);
    // GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);

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
    GX_SetTevColor(
        GX_TEVREG1, (GXColor){0xFF, 0xFF, 0xFF, m_alpha}
    ); // White color
    GX_SetTevColor(
        GX_TEVREG2, (GXColor){0x00, 0x00, 0x00, 0x00}
    ); // Alpha color

    GX_InvalidateTexAll();

    // Blending
    GX_SetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_SET);

    GX_SetColorUpdate(GX_TRUE);
    GX_SetAlphaUpdate(GX_TRUE);

    // Draw picture
    GX_LoadTexObj(&s_texObj, GX_TEXMAP0);

    GX_Begin(GX_QUADS, GX_VTXFMT0, 4);
    {
        GX_Position3f32(m_x, m_y, 0); // top left
        GX_TexCoord2f32(0.0, 1.0);

        GX_Position3f32(m_x + m_width, m_y, 0); // top right
        GX_TexCoord2f32(1.0, 1.0);

        GX_Position3f32(m_x + m_width, m_y + m_height, 0); // bottom right
        GX_TexCoord2f32(1.0, 0.0);

        GX_Position3f32(m_x, m_y + m_height, 0); // bottom left
        GX_TexCoord2f32(0.0, 0.0);
    }
}

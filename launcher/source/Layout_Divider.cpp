#include "Layout_Divider.hpp"
#include <cstdio>
#include <ogc/gx.h>

void Layout_Divider::Init()
{
    m_width = 600.0 * 2;
    m_height = 3.0f;

    m_x = -600.0;
    m_y = -120.0;
}

void Layout_Divider::Calc()
{
}

static void DrawRectangle(float x, float y, float width, float height)
{
    GX_SetColorUpdate(GX_TRUE);
    GX_SetAlphaUpdate(GX_TRUE);

    GX_Begin(GX_QUADS, GX_VTXFMT0, 4);
    {
        GX_Position3f32(x, y, -10.0f); // top left
        GX_Position3f32(x + width, y, -10.0f); // top right
        GX_Position3f32(x + width, y - height, -10.0f); // bottom right
        GX_Position3f32(x, y - height, -10.0f); // bottom left
    }
    GX_End();
}

void Layout_Divider::Draw()
{
    if (!m_visible || m_alpha == 0) {
        return;
    }

    // Set up for drawing f32 quads
    GX_ClearVtxDesc();
    GX_SetVtxDesc(GX_VA_POS, GX_DIRECT);
    GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);

    // Disable texture and indirect
    GX_SetNumTexGens(0);
    GX_SetNumIndStages(0);

    // Set material channels
    GX_SetNumChans(1);
    GX_SetChanCtrl(
        GX_COLOR0A0, GX_DISABLE, GX_SRC_REG, GX_SRC_REG, 0, GX_DF_NONE,
        GX_AF_NONE
    );

    // Setup TEV
    GX_SetNumTevStages(1);
    GX_SetTevOp(GX_TEVSTAGE0, GX_PASSCLR);
    GX_SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORDNULL, GX_TEXMAP_NULL, GX_COLOR0A0);

    // Set line color
    GX_SetChanMatColor(GX_COLOR0A0, (GXColor){0xF0, 0xF0, 0xF0, m_alpha});

    // Draw line
    DrawRectangle(m_x, m_y, m_width, m_height);

    // Set background color
    GX_SetChanMatColor(GX_COLOR0A0, (GXColor){0x20, 0x20, 0x20, m_alpha});

    // Draw background
    DrawRectangle(m_x, m_y - m_height, m_width, 500.0);
}
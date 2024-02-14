#include "Layout_LoadingIcon.hpp"
#include <ogc/gx.h>

void Layout_LoadingIcon::Init()
{
    m_frame = 0;
    m_x = 0.0f;
    m_y = 0.0f;
    m_z = 0.0f;

    m_width = 100.0f;
    m_height = 100.0f;

    m_alpha = 0xFF;
}

void Layout_LoadingIcon::Calc()
{
    if (m_animate) {
        m_frame++;
    }
}

static void DrawRectangle(float x, float y, float width, float height)
{
    GX_SetColorUpdate(GX_TRUE);
    GX_SetAlphaUpdate(GX_TRUE);

    GX_Begin(GX_QUADS, GX_VTXFMT0, 4);
    {
        GX_Position3f32(x, y, 0.0f); // top left
        GX_Position3f32(x + width, y, 0.0f); // top right
        GX_Position3f32(x + width, y + height, 0.0f); // bottom right
        GX_Position3f32(x, y + height, 0.0f); // bottom left
    }
    GX_End();
}

void Layout_LoadingIcon::Draw()
{
    if (!m_visible || m_alpha == 0) {
        return;
    }

    // Set up for drawing f32 quads
    GX_ClearVtxDesc();
    GX_SetVtxDesc(GX_VA_POS, GX_DIRECT);
    GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);

    // Set material channels
    GX_SetNumChans(1);
    GX_SetChanMatColor(GX_COLOR0A0, (GXColor){0xF0, 0xF0, 0xF0, m_alpha});
    GX_SetChanCtrl(
        GX_COLOR0A0, GX_DISABLE, GX_SRC_REG, GX_SRC_REG, 0, GX_DF_NONE,
        GX_AF_NONE
    );

    // Disable texture and indirect
    GX_SetNumTexGens(0);
    GX_SetNumIndStages(0);

    // Setup TEV
    GX_SetNumTevStages(1);
    GX_SetTevOp(GX_TEVSTAGE0, GX_PASSCLR);
    GX_SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORDNULL, GX_TEXMAP_NULL, GX_COLOR0A0);

    // Setup blending
    if (m_alpha == 0xFF) {
        // No blending
        GX_SetBlendMode(GX_BM_NONE, GX_BL_ONE, GX_BL_ZERO, GX_LO_SET);
    } else {
        // Blending
        GX_SetBlendMode(
            GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_SET
        );
    }

    u8 current = 0xFF;

    if (m_animate) {
        static constexpr u32 ANIM_DELAY = 5;
        static constexpr u8 LOADING_ORDER[8] = {0, 1, 2, 5, 8, 7, 6, 3};

        current =
            LOADING_ORDER[(-m_frame / ANIM_DELAY) % sizeof(LOADING_ORDER)];
    }

    // Draw 9 rectangles in a 3x3 grid
    for (int i = 0; i < 9; i++) {
        float x = m_x + (i % 3) * (m_width / 3.0f);
        float y = m_y + (i / 3) * (m_height / 3.0f);

        x -= m_width / 2.0f;
        y -= m_height / 2.0f;

        if (i == current) {
            continue;
        }

        DrawRectangle(x, y, m_width / 5.0f, m_height / 5.0f);
    }
}
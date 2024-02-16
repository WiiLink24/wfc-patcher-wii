#include "Layout_Divider.hpp"
#include "Scene.hpp"
#include <cstdio>
#include <ogc/gx.h>

void Layout_Divider::Init()
{
    auto rect = Scene::GetProjectionRect();

    m_width = rect.right - rect.left + 300.0 * 2;
    m_height = 3.0;

    m_x = rect.left - 300.0;
    m_y = -130.0;
}

void Layout_Divider::Calc()
{
    m_animFrame++;

    if (m_animFrame > ANIM_LINE_FRAMES) {
        m_animFrame = ANIM_LINE_FRAMES;
    }
}

void Layout_Divider::Draw()
{
    DrawBack();
    DrawFront();
}

static void SetupGX()
{
    // Set up for drawing f32 quads
    GX_ClearVtxDesc();
    GX_SetVtxDesc(GX_VA_POS, GX_DIRECT);
    GX_SetVtxDesc(GX_VA_CLR0, GX_DIRECT);
    GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
    GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);

    // Disable texture and indirect
    GX_SetNumTexGens(0);
    GX_SetNumIndStages(0);

    // Set material channels
    GX_SetNumChans(1);
    GX_SetChanCtrl(
        GX_COLOR0A0, GX_DISABLE, GX_SRC_VTX, GX_SRC_VTX, 0, GX_DF_NONE,
        GX_AF_NONE
    );

    // Setup TEV
    GX_SetNumTevStages(1);
    GX_SetTevOp(GX_TEVSTAGE0, GX_PASSCLR);
    GX_SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORDNULL, GX_TEXMAP_NULL, GX_COLOR0A0);

    GX_InvalidateTexAll();

    GX_SetColorUpdate(GX_TRUE);
    GX_SetAlphaUpdate(GX_TRUE);

    GX_SetColorUpdate(GX_TRUE);
    GX_SetAlphaUpdate(GX_TRUE);
}

void Layout_Divider::DrawBack()
{
    if (!m_visible || m_alpha == 0) {
        return;
    }

    float x = m_x;
    float animStep = m_width / float(ANIM_LINE_FRAMES);
    if (m_animState == AnimState::IN) {
        x += m_width - m_animFrame * animStep;
    } else if (m_animState == AnimState::OUT_BACK) {
        x += m_animFrame * animStep;
    } else if (m_animState == AnimState::OUT) {
        x -= m_animFrame * animStep;
    } else {
        x = m_x;
    }

    const u8 bgColor = 0x20;

    SetupGX();

    GX_Begin(GX_QUADS, GX_VTXFMT0, 8);
    {
        // Line
        GX_Position3f32(x, m_y, -5.0f); // top left
        GX_Color4u8(0xF0, 0xF0, 0xF0, m_alpha);
        GX_Position3f32(x + m_width, m_y, -5.0f); // top right
        GX_Color4u8(0xF0, 0xF0, 0xF0, m_alpha);
        GX_Position3f32(x + m_width, m_y - m_height, -5.0f); // bottom right
        GX_Color4u8(0xF0, 0xF0, 0xF0, m_alpha);
        GX_Position3f32(x, m_y - m_height, -10.0f); // bottom left
        GX_Color4u8(0xF0, 0xF0, 0xF0, m_alpha);

        // Background solid
        GX_Position3f32(m_x, m_y - m_height, -20.0f); // top left
        GX_Color4u8(bgColor, bgColor, bgColor, m_alpha);
        GX_Position3f32(m_x + m_width, m_y - m_height, -20.0f); // top right
        GX_Color4u8(bgColor, bgColor, bgColor, m_alpha);
        GX_Position3f32(m_x + m_width, m_y - 500.0, -20.0f); // bottom right
        GX_Color4u8(bgColor, bgColor, bgColor, m_alpha);
        GX_Position3f32(m_x, m_y - 500.0, -20.0f); // bottom left
        GX_Color4u8(bgColor, bgColor, bgColor, m_alpha);
    }
    GX_End();
}

void Layout_Divider::DrawFront()
{
    if (!m_visible || m_alpha == 0) {
        return;
    }

    float x = m_x;
    float animStep = m_width / float(ANIM_LINE_FRAMES);
    if (m_animState == AnimState::IN) {
        x += m_width - m_animFrame * animStep;
    } else if (m_animState == AnimState::OUT_BACK) {
        x += m_animFrame * animStep;
    } else if (m_animState == AnimState::OUT) {
        x -= m_animFrame * animStep;
    } else {
        return;
    }

    const float fadeWidth = 300.0;

    SetupGX();

    // Draw background fade
    if (m_animState == AnimState::IN || m_animState == AnimState::OUT_BACK) {
        GX_Begin(GX_QUADS, GX_VTXFMT0, 8);
        {
            // Draw black to the left of the fade
            GX_Position3f32(m_x, m_y - m_height, -10.0f); // top left
            GX_Color4u8(0, 0, 0, m_alpha);
            GX_Position3f32(x, m_y - m_height, -10.0f); // top right
            GX_Color4u8(0, 0, 0, m_alpha);
            GX_Position3f32(x, m_y - 500.0, -10.0f); // bottom right
            GX_Color4u8(0, 0, 0, m_alpha);
            GX_Position3f32(m_x, m_y - 500.0, -10.0f); // bottom left
            GX_Color4u8(0, 0, 0, m_alpha);

            // Draw fade
            GX_Position3f32(x, m_y - m_height, -10.0f); // top left
            GX_Color4u8(0, 0, 0, m_alpha);
            GX_Position3f32(x + fadeWidth, m_y - m_height, -10.0f); // top right
            GX_Color4u8(0, 0, 0, 0);
            GX_Position3f32(x + fadeWidth, m_y - 500.0, -10.0f); // bottom right
            GX_Color4u8(0, 0, 0, 0);
            GX_Position3f32(x, m_y - 500.0, -10.0f); // bottom left
            GX_Color4u8(0, 0, 0, m_alpha);
        }
        GX_End();
    } else if (m_animState == AnimState::OUT) {
        GX_Begin(GX_QUADS, GX_VTXFMT0, 8);
        {
            // Draw black to the right of the fade
            GX_Position3f32(x + m_width, m_y - m_height, -10.0f); // top left
            GX_Color4u8(0, 0, 0, m_alpha);
            GX_Position3f32(
                x + m_width * 2, m_y - m_height, -10.0f
            ); // top right
            GX_Color4u8(0, 0, 0, m_alpha);
            GX_Position3f32(
                x + m_width * 2, m_y - 500.0, -10.0f
            ); // bottom right
            GX_Color4u8(0, 0, 0, m_alpha);
            GX_Position3f32(x + m_width, m_y - 500.0, -10.0f); // bottom left
            GX_Color4u8(0, 0, 0, m_alpha);

            // Draw fade
            GX_Position3f32(
                x + m_width - fadeWidth, m_y - m_height, -10.0f
            ); // top left
            GX_Color4u8(0, 0, 0, 0);
            GX_Position3f32(x + m_width, m_y - m_height, -10.0f); // top right
            GX_Color4u8(0, 0, 0, m_alpha);
            GX_Position3f32(x + m_width, m_y - 500.0, -10.0f); // bottom right
            GX_Color4u8(0, 0, 0, m_alpha);
            GX_Position3f32(
                x + m_width - fadeWidth, m_y - 500.0, -10.0f
            ); // bottom left
            GX_Color4u8(0, 0, 0, 0);
        }
        GX_End();
    }
}

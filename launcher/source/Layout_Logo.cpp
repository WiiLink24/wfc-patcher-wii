#include "Layout_Logo.hpp"
#include "Assets.hpp"
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <ogc/gx.h>

static GXTexObj s_texObj = {};

void Layout_Logo::Init()
{
    s_texObj = Assets::GetTexture("wfc-logo.tpl", &m_width, &m_height);
}

void Layout_Logo::Calc()
{
    static constexpr GXColor VTX_COLOR_LIST[] = {
        {0x66, 0x66, 0xA9}, // Blue
        {0x66, 0x66, 0xED}, // Light Blue
        {0x66, 0xA9, 0x66}, // Green
        {0x66, 0xED, 0x66}, // Light Green
        {0x66, 0xA9, 0xA9}, // Cyan
        {0x66, 0xA9, 0xED}, // Light Cyan
        {0x66, 0xED, 0x66}, // Green
        {0x66, 0xED, 0xA9}, // Light Green
        {0xA9, 0xED, 0x66}, // Light Green
        {0xED, 0x66, 0x66}, // Red
        {0xED, 0x66, 0xA9}, // Light Red
        {0xED, 0x66, 0xED}, // Magenta
        {0xED, 0xA9, 0x66}, // Orange
        {0xED, 0xED, 0x66}, // Yellow
    };

    static constexpr u8 VTX_COLOR_COUNT =
        sizeof(VTX_COLOR_LIST) / sizeof(GXColor);

    static constexpr s32 FADE_FRAMES = 240;

    for (u32 i = 0; i < 4; i++) {
        VtxColorContext* ctx = &m_vtxColor[i];

        if (ctx->m_time == -1) {
            ctx->m_color = std::rand() % VTX_COLOR_COUNT;
            ctx->m_nextColor = std::rand() % VTX_COLOR_COUNT;
            ctx->m_time = FADE_FRAMES;
            ctx->m_end = FADE_FRAMES + i * (FADE_FRAMES / 4);
        } else if (ctx->m_time < ctx->m_end) {
            ctx->m_time++;
        } else {
            if (i != 2) {
                ctx->m_nextColor = std::rand() % VTX_COLOR_COUNT;
            } else {
                // 2 is the opposite of 0
                ctx->m_nextColor =
                    (m_vtxColor[0].m_nextColor + VTX_COLOR_COUNT / 2) %
                    VTX_COLOR_COUNT;
            }
            ctx->m_time = 0;
            ctx->m_end = FADE_FRAMES;
        }

        if (ctx->m_time < FADE_FRAMES) {
            static constexpr double PI = 3.14159265359;
            double point =
                (std::sin(
                     double(ctx->m_time) / double(FADE_FRAMES) * PI - PI / 2.0
                 ) +
                 1.0) /
                2.0;

            ctx->r =
                u8(double(VTX_COLOR_LIST[ctx->m_nextColor].r) * point +
                   double(VTX_COLOR_LIST[ctx->m_color].r) * (1.0 - point));
            ctx->g =
                u8(double(VTX_COLOR_LIST[ctx->m_nextColor].g) * point +
                   double(VTX_COLOR_LIST[ctx->m_color].g) * (1.0 - point));
            ctx->b =
                u8(double(VTX_COLOR_LIST[ctx->m_nextColor].b) * point +
                   double(VTX_COLOR_LIST[ctx->m_color].b) * (1.0 - point));
            continue;
        }

        if (ctx->m_time == FADE_FRAMES) {
            ctx->m_color = ctx->m_nextColor;
        }

        ctx->r = VTX_COLOR_LIST[ctx->m_color].r;
        ctx->g = VTX_COLOR_LIST[ctx->m_color].g;
        ctx->b = VTX_COLOR_LIST[ctx->m_color].b;
    }

    // Alpha fade
    static constexpr s32 ALPHA_FADE_FRAMES = 10;

    if (m_animState != AnimState::NONE && m_animFrame < ALPHA_FADE_FRAMES) {
        m_animFrame++;

        m_alpha = u8(double(m_animFrame) / double(ALPHA_FADE_FRAMES) * 255.0);

        if (m_animState == AnimState::OUT) {
            m_alpha = 255 - m_alpha;
        }
    }
}

void Layout_Logo::Draw()
{
    if (!m_visible || m_alpha == 0 || m_width == 0 || m_height == 0) {
        return;
    }

    // Set up for drawing f32 quads with tex map
    GX_ClearVtxDesc();
    GX_SetVtxDesc(GX_VA_POS, GX_DIRECT);
    GX_SetVtxDesc(GX_VA_CLR0, GX_DIRECT);
    GX_SetVtxDesc(GX_VA_TEX0, GX_DIRECT);
    GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
    GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);
    GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);

    // Set material channels, TEV, texture
    GX_SetNumChans(1);
    GX_SetNumTexGens(1);
    GX_SetChanCtrl(
        GX_COLOR0A0, GX_DISABLE, GX_SRC_VTX, GX_SRC_VTX, 0, GX_DF_NONE,
        GX_AF_NONE
    );
    GX_SetTevOp(GX_TEVSTAGE0, GX_REPLACE);
    GX_SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
    GX_SetTevColorIn(
        GX_TEVSTAGE0, GX_CC_RASC, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO
    );
    GX_SetTevAlphaIn(
        GX_TEVSTAGE0, GX_CC_RASA, GX_CA_RASA, GX_CA_TEXA, GX_CA_ZERO
    );
    GX_SetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY);

    GX_InvalidateTexAll();

    // Blending
    GX_SetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_SET);

    GX_SetColorUpdate(GX_TRUE);
    GX_SetAlphaUpdate(GX_TRUE);

    // Draw picture
    GX_LoadTexObj(&s_texObj, GX_TEXMAP0);

    GX_Begin(GX_QUADS, GX_VTXFMT0, 8);
    {
        // WiiLink
        GX_Position3f32(m_x, m_y, 0); // top left
        GX_Color4u8(0xFF, 0xFF, 0xFF, m_alpha);
        GX_TexCoord2f32(0.0, 1.0);

        GX_Position3f32(m_x + m_width * 0.55, m_y, 0); // top right
        GX_Color4u8(0xFF, 0xFF, 0xFF, m_alpha);
        GX_TexCoord2f32(0.55, 1.0);

        GX_Position3f32(
            m_x + m_width * 0.55, m_y + m_height, 0
        ); // bottom right
        GX_Color4u8(0xFF, 0xFF, 0xFF, m_alpha);
        GX_TexCoord2f32(0.55, 0.0);

        GX_Position3f32(m_x, m_y + m_height, 0); // bottom left
        GX_Color4u8(0xFF, 0xFF, 0xFF, m_alpha);
        GX_TexCoord2f32(0.0, 0.0);

        // WFC
        GX_Position3f32(m_x + m_width * 0.55, m_y, 0); // top left
        GX_Color4u8(m_vtxColor[0].r, m_vtxColor[0].g, m_vtxColor[0].b, m_alpha);
        GX_TexCoord2f32(0.55, 1.0);

        GX_Position3f32(m_x + m_width, m_y, 0); // top right
        GX_Color4u8(m_vtxColor[1].r, m_vtxColor[1].g, m_vtxColor[1].b, m_alpha);
        GX_TexCoord2f32(1.0, 1.0);

        GX_Position3f32(m_x + m_width, m_y + m_height, 0); // bottom right
        GX_Color4u8(m_vtxColor[2].r, m_vtxColor[2].g, m_vtxColor[2].b, m_alpha);
        GX_TexCoord2f32(1.0, 0.0);

        GX_Position3f32(m_x + m_width * 0.55, m_y + m_height, 0); // bottom left
        GX_Color4u8(m_vtxColor[3].r, m_vtxColor[3].g, m_vtxColor[3].b, m_alpha);
        GX_TexCoord2f32(0.55, 0.0);
    }
}

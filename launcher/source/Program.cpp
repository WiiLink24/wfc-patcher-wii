#include "Apploader.hpp"
#include "DI.hpp"
#include "Layout_Divider.hpp"
#include "Layout_LoadingIcon.hpp"
#include "Layout_TextBox.hpp"
#include "Util.hpp"
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ogc/cache.h>
#include <ogc/conf.h>
#include <ogc/consol.h>
#include <ogc/gu.h>
#include <ogc/gx.h>
#include <ogc/system.h>
#include <ogc/video.h>
#include <wiiuse/wpad.h>

static void* s_xfb[2] = {nullptr, nullptr};
static u32 s_currXfb = 0;
static void* s_consoleXfb = nullptr;
static GXRModeObj s_rmode = {};
static s32 s_aspectRatio = 0;

/**
 * Get the render mode to use for the program.
 */
GXRModeObj GetRenderMode()
{
    GXRModeObj rmode = {};

    bool dtv = VIDEO_HaveComponentCable() != 0;
    bool pal60 = CONF_GetEuRGB60() == 1;
    bool progressive = CONF_GetProgressiveScan() == 1;
    u8 aspect = CONF_GetAspectRatio();
    u32 mode = CONF_GetVideo();

    s_aspectRatio = aspect;

    switch (mode) {
    case VI_NTSC:
    default:
        if (dtv && progressive) {
            // NTSC 480p
            rmode = {
                .viTVMode = (VI_NTSC << 2) | 2,
                .fbWidth = 608,
                .efbHeight = 456,
                .xfbHeight = 456,
                .viXOrigin = u16(aspect ? 17 : 25),
                .viYOrigin = 12,
                .viWidth = u16(aspect ? 686 : 670),
                .viHeight = 456,
                .xfbMode = 0,
                .field_rendering = 0,
                .aa = 0,
                .sample_pattern = {6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
                                   6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6},
                .vfilter = {0, 0, 21, 22, 21, 0, 0},
            };
            break;
        } else {
            // NTSC 480i
            rmode = {
                .viTVMode = VI_NTSC << 2,
                .fbWidth = 608,
                .efbHeight = 456,
                .xfbHeight = 456,
                .viXOrigin = u16(aspect ? 17 : 25),
                .viYOrigin = 12,
                .viWidth = u16(aspect ? 686 : 670),
                .viHeight = 456,
                .xfbMode = 1,
                .field_rendering = 0,
                .aa = 0,
                .sample_pattern = {6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
                                   6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6},
                .vfilter = {7, 7, 12, 12, 12, 7, 7},
            };
            break;
        }

    case VI_PAL:
    case VI_EURGB60:
        if (dtv && progressive) {
            // PAL60 progressive
            rmode = {
                .viTVMode = (VI_EURGB60 << 2) | 2,
                .fbWidth = 608,
                .efbHeight = 456,
                .xfbHeight = 456,
                .viXOrigin = u16(aspect ? 17 : 25),
                .viYOrigin = 12,
                .viWidth = u16(aspect ? 686 : 670),
                .viHeight = 456,
                .xfbMode = 0,
                .field_rendering = 0,
                .aa = 0,
                .sample_pattern = {6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
                                   6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6},
                .vfilter = {0, 0, 21, 22, 21, 0, 0},
            };
            break;
        }

        if (pal60) {
            // PAL60 interlaced
            rmode = {
                .viTVMode = VI_EURGB60 << 2,
                .fbWidth = 608,
                .efbHeight = 456,
                .xfbHeight = 456,
                .viXOrigin = u16(aspect ? 17 : 25),
                .viYOrigin = 12,
                .viWidth = u16(aspect ? 686 : 670),
                .viHeight = 456,
                .xfbMode = 1,
                .field_rendering = 0,
                .aa = 0,
                .sample_pattern = {6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
                                   6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6},
                .vfilter = {7, 7, 12, 12, 12, 7, 7},
            };
            break;
        } else {
            // PAL50 interlaced
            rmode = {
                .viTVMode = VI_PAL << 2,
                .fbWidth = 608,
                .efbHeight = 456,
                .xfbHeight = 542,
                .viXOrigin = u16(aspect ? 19 : 27),
                .viYOrigin = 16,
                .viWidth = u16(aspect ? 682 : 666),
                .viHeight = 542,
                .xfbMode = 1,
                .field_rendering = 0,
                .aa = 0,
                .sample_pattern = {6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
                                   6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6},
                .vfilter = {7, 7, 12, 12, 12, 7, 7},
            };
            break;
        }

#if 0
        // Not sure if this is needed, PAL-M seems to work fine when Mario Kart
        // Wii tries to switch the video mode to NTSC, so I'm going to stick to
        // that
    case VI_MPAL:
        if (dtv && progressive) {
            // PAL-M 480p
            rmode = {
                .viTVMode = (VI_MPAL << 2) | 2,
                .fbWidth = 608,
                .efbHeight = 456,
                .xfbHeight = 456,
                .viXOrigin = u16(aspect ? 17 : 25),
                .viYOrigin = 12,
                .viWidth = u16(aspect ? 686 : 670),
                .viHeight = 456,
                .xfbMode = 0,
                .field_rendering = 0,
                .aa = 0,
                .sample_pattern = {6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
                                   6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6},
                .vfilter = {0, 0, 21, 22, 21, 0, 0},
            };
            break;
        } else {
            // PAL-M 480i
            rmode = {
                .viTVMode = VI_MPAL << 2,
                .fbWidth = 608,
                .efbHeight = 456,
                .xfbHeight = 456,
                .viXOrigin = u16(aspect ? 17 : 25),
                .viYOrigin = 12,
                .viWidth = u16(aspect ? 686 : 670),
                .viHeight = 456,
                .xfbMode = 1,
                .field_rendering = 0,
                .aa = 0,
                .sample_pattern = {6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
                                   6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6},
                .vfilter = {7, 7, 12, 12, 12, 7, 7},
            };
            break;
        }
#endif
    }

    return rmode;
}

static void* AllocMEM1(s32 size)
{
    InterruptsLock lock{};

    u8* start = reinterpret_cast<u8*>(SYS_GetArenaLo());
    u8* end = reinterpret_cast<u8*>(SYS_GetArenaHi());

    assert(start != nullptr);
    assert(end != nullptr);
    assert(size <= (end - start));

    start = AlignUp(start, 32);
    SYS_SetArena1Lo(AlignUp(start + size, 32));

    DCZeroRange(start, size);
    return start;
}

struct Rect {
    float left;
    float top;
    float right;
    float bottom;
};

static Rect GetProjectionRect()
{
    if (s_aspectRatio == 1) {
        return Rect{-416, 228, 416, -228};
    } else {
        return Rect{-304, 228, 304, -228};
    }
}

static sys_fontheader* s_fontHeader = nullptr;
static Layout_LoadingIcon s_loadingIcon;
static Layout_TextBox s_textBox;
static Layout_Divider s_divider;

void LayoutInit()
{
    s_fontHeader =
        reinterpret_cast<sys_fontheader*>(AllocMEM1(SYS_FONTSIZE_SJIS));
    s32 ret = SYS_InitFont(s_fontHeader);
    assert(ret == 1);

    s_loadingIcon.Init();
    s_loadingIcon.m_width = 50.0;
    s_loadingIcon.m_height = 50.0;
    s_loadingIcon.m_x = GetProjectionRect().right - 56.0;
    s_loadingIcon.m_y = -176.0;
    s_loadingIcon.StopAnimation();

    s_textBox.Init(s_fontHeader);
    s_textBox.SetText(L"Please insert a disc.");
    s_textBox.SetFontSize(1.5f);
    s_textBox.SetFontColor((GXColor){0xFF, 0xFF, 0xFF, 0xFF});
    s_textBox.SetMonospace(false);
    s_textBox.SetKerning(-3.0);
    s_textBox.SetLeading(6.0);
    s_textBox.m_width = 500.0;
    s_textBox.m_height = 400.0;
    s_textBox.m_x = 0;
    s_textBox.m_y = -176.0;
    s_textBox.m_alpha = 0xFF;

    s_divider.Init();
}

void LayoutCalc()
{
    auto state = Apploader::GetState();

    switch (state) {
    case Apploader::State::INITIALIZING:
        s_loadingIcon.StopAnimation();
        s_textBox.m_visible = false;
        break;

    case Apploader::State::DISC_SPINUP:
        s_loadingIcon.StartAnimation();
        s_textBox.m_visible = true;
        s_textBox.SetText(L"Starting up the disc drive...");
        break;

    case Apploader::State::READING_DISC:
        s_loadingIcon.StartAnimation();
        s_textBox.m_visible = true;
        s_textBox.SetText(L"Reading the disc...");
        break;

    case Apploader::State::LAUNCHING:
        s_loadingIcon.StartAnimation();
        s_textBox.m_visible = true;
        s_textBox.SetText(L"Launching the game...");
        break;

    case Apploader::State::NO_DISC:
        s_loadingIcon.StopAnimation();
        s_textBox.m_visible = true;
        s_textBox.SetText(L"Please insert a disc.");
        break;

    case Apploader::State::READ_ERROR:
        s_loadingIcon.StopAnimation();
        s_textBox.m_visible = true;
        s_textBox.SetText(L"Unable the read the disc.");
        break;

    case Apploader::State::UNSUPPORTED_GAME:
        s_loadingIcon.StopAnimation();
        s_textBox.m_visible = true;
        s_textBox.SetText(L"This game is not supported by WiiLink WFC.");
        break;

    case Apploader::State::FATAL_ERROR:
        s_loadingIcon.StopAnimation();
        s_textBox.m_visible = true;
        s_textBox.SetText(L"An error has occurred");
        break;

    case Apploader::State::SHUTTING_DOWN:
        break;
    }

    s_loadingIcon.Calc();
    s_textBox.Calc();
    s_divider.Calc();
}

void LayoutDraw()
{
    s_divider.Draw();
    s_loadingIcon.Draw();
    s_textBox.Draw();
}

enum class ShutdownType {
    NONE,
    POWER_OFF,
    EXIT,
};

static ShutdownType s_shutdownType = ShutdownType::NONE;

int main(int argc, char** argv)
{
    (void) argc;
    (void) argv;

    LWP_SetThreadPriority(LWP_GetSelf(), 100);

    // Initialize video
    VIDEO_Init();
    s_rmode = GetRenderMode();
    s_consoleXfb = MEM_K0_TO_K1(SYS_AllocateFramebuffer(&s_rmode));
    s_xfb[0] = MEM_K0_TO_K1(SYS_AllocateFramebuffer(&s_rmode));
    s_xfb[1] = MEM_K0_TO_K1(SYS_AllocateFramebuffer(&s_rmode));
    VIDEO_Configure(&s_rmode);

    // Initialize LibOGC console
    CON_Init(
        s_consoleXfb, 20, 20, s_rmode.fbWidth, s_rmode.xfbHeight,
        s_rmode.fbWidth * VI_DISPLAY_PIX_SZ
    );
    // VIDEO_SetNextFramebuffer(s_consoleXfb);
    // VIDEO_SetBlack(false);
    // VIDEO_Flush();

    // Adjust to row 2 column 0
    std::printf("\x1b[%d;%dH", 2, 0);

    // Print logo
    std::printf("WiiLink WFC Launcher\n\n");

    // Initialize DI
    if (!DI::Init()) {
        std::printf("Failed to initialize DI\n");
        return EXIT_FAILURE;
    }

    // Initialize WPAD
    WPAD_Init();

    // Set event handlers for face buttons
    STM_RegisterEventHandler([](u32 event) {
        if (event == STM_EVENT_RESET) {
            s_shutdownType = ShutdownType::EXIT;
            Apploader::ShutdownAsync();
        }

        if (event == STM_EVENT_POWER) {
            s_shutdownType = ShutdownType::POWER_OFF;
            Apploader::ShutdownAsync();
        }
    });

    Apploader::StartThread();

    // Initialize GX
    GX_Init(AllocMEM1(0x80000), 0x80000);

    GX_SetViewport(0, 0, s_rmode.fbWidth, s_rmode.efbHeight, 0, 1);
    GX_SetScissor(0, 0, s_rmode.fbWidth, s_rmode.efbHeight);

    float factor = GX_GetYScaleFactor(s_rmode.efbHeight, s_rmode.xfbHeight);
    u16 lines = GX_SetDispCopyYScale(factor);

    GX_SetDispCopySrc(0, 0, s_rmode.fbWidth, s_rmode.xfbHeight);
    GX_SetDispCopyDst(s_rmode.fbWidth, lines);
    GX_SetCopyFilter(s_rmode.aa, s_rmode.sample_pattern, 0, s_rmode.vfilter);
    GX_SetPixelFmt(0, 0);

    GX_SetViewport(0, 0, s_rmode.fbWidth, s_rmode.efbHeight, 0, 1);

    bool firstFrame = true;
    const GXColor bgColor = {0x00, 0x00, 0x00, 0x00};

    LayoutInit();

    // Main loop
    while (true) {
        if (Apploader::GetState() == Apploader::State::SHUTTING_DOWN) {
            s_divider.StartFadeOutBack();

            if (s_divider.IsFadeDone()) {
                break;
            }
        }

        WPAD_ScanPads();

        // Check for HOME button press
        for (u32 i = 0; i < 4; i++) {
            if (s_shutdownType == ShutdownType::NONE &&
                WPAD_ButtonsDown(i) & WPAD_BUTTON_HOME) {
                std::printf("Exiting...\n");

                s_shutdownType = ShutdownType::EXIT;
                Apploader::ShutdownAsync();
            }
        }

        LayoutCalc();

        VIDEO_WaitVSync();

        GX_InvVtxCache();
        GX_InvalidateTexAll();

        {
            float mtx[4][4];
            auto rect = GetProjectionRect();
            guOrtho(mtx, rect.top, rect.bottom, rect.left, rect.right, 0, 500);
            GX_LoadProjectionMtx(mtx, GX_ORTHOGRAPHIC);
        }

        {
            float mtx[3][4];
            guMtxIdentity(mtx);
            GX_LoadPosMtxImm(mtx, 0);
            GX_SetCurrentMtx(0);
        }

        GX_SetLineWidth(6, 0);
        GX_SetPointSize(6, 0);
        GX_SetCullMode(0);
        GX_SetZMode(GX_TRUE, GX_LEQUAL, GX_TRUE);

        LayoutDraw();

        GX_SetCopyClear(bgColor, 0xFFFFFF);
        GX_SetZMode(1, 7, 1);

        GX_SetAlphaUpdate(1);
        GX_SetColorUpdate(1);

        GX_CopyDisp(s_xfb[s_currXfb], 1);
        GX_DrawDone();

        VIDEO_SetNextFramebuffer(s_xfb[s_currXfb]);

        if (firstFrame) {
            VIDEO_SetBlack(false);
            firstFrame = false;
        }

        VIDEO_Flush();

        // Swap framebuffers
        s_currXfb ^= 1;
    }

    VIDEO_SetBlack(true);
    VIDEO_Flush();
    VIDEO_WaitVSync();

    Apploader::Shutdown();

    if (s_shutdownType == ShutdownType::POWER_OFF) {
        SYS_ResetSystem(SYS_POWEROFF, 0, 0);
    }

    return EXIT_SUCCESS;
}
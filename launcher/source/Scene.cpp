#include "Scene.hpp"
#include "Apploader.hpp"
#include "Layout.hpp"
#include "Layout_Divider.hpp"
#include "Layout_LoadingIcon.hpp"
#include "Layout_Logo.hpp"
#include "Layout_TextBox.hpp"
#include "Util.hpp"
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ogc/conf.h>
#include <ogc/gu.h>
#include <ogc/gx.h>
#include <ogc/lwp.h>
#include <ogc/system.h>
#include <wchar.h>
#include <wiiuse/wpad.h>

static void* s_xfb[2] = {nullptr, nullptr};
static u32 s_currXfb = 0;
static sys_fontheader* s_fontHeader = nullptr;
static Layout_LoadingIcon s_loadingIcon;
static Layout_TextBox s_textBox;
static Layout_Divider s_divider;
static Layout_Logo s_logo;
static u8 s_aspectRatio = 0;
static Scene::ShutdownType s_shutdownType = Scene::ShutdownType::NONE;

Rect Scene::GetProjectionRect()
{
    if (s_aspectRatio == 1) {
        return Rect{-416, 228, 416, -228};
    } else {
        return Rect{-304, 228, 304, -228};
    }
}

void Scene::Init(GXRModeObj* rmode)
{
    s_xfb[0] = MEM_K0_TO_K1(SYS_AllocateFramebuffer(rmode));
    s_xfb[1] = MEM_K0_TO_K1(SYS_AllocateFramebuffer(rmode));
    s_currXfb = 0;

    s_aspectRatio = CONF_GetAspectRatio();

    // Initialize GX
    GX_Init(AllocMEM1(0x80000), 0x80000);

    GX_SetViewport(0, 0, rmode->fbWidth, rmode->efbHeight, 0, 1);
    GX_SetScissor(0, 0, rmode->fbWidth, rmode->efbHeight);

    float factor = GX_GetYScaleFactor(rmode->efbHeight, rmode->xfbHeight);
    u16 lines = GX_SetDispCopyYScale(factor);

    GX_SetDispCopySrc(0, 0, rmode->fbWidth, rmode->efbHeight);
    GX_SetDispCopyDst(rmode->fbWidth, lines);
    GX_SetCopyFilter(rmode->aa, rmode->sample_pattern, 0, rmode->vfilter);
    GX_SetPixelFmt(0, 0);

    GX_SetViewport(0, 0, rmode->fbWidth, rmode->efbHeight, 0, 1);
}

static void LayoutInit()
{
    s_fontHeader =
        reinterpret_cast<sys_fontheader*>(AllocMEM1(SYS_FONTSIZE_SJIS));
    s32 ret = SYS_InitFont(s_fontHeader);
    assert(ret == 1);

    s_loadingIcon.Init();
    s_loadingIcon.m_width = 42.0;
    s_loadingIcon.m_height = 42.0;
    s_loadingIcon.m_x = Scene::GetProjectionRect().right - 56.0;
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

    s_logo.Init();
    s_logo.m_x = Scene::GetProjectionRect().right - 32.0 - 251.0;
    s_logo.m_y = 130.0;
}

static void LayoutCalc()
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
        s_textBox.SetNextText(L"Starting up the disc drive...");
        break;

    case Apploader::State::READING_DISC:
        s_loadingIcon.StartAnimation();
        s_textBox.m_visible = true;
        s_textBox.SetNextText(L"Reading the disc...");
        break;

    case Apploader::State::LAUNCHING:
        s_loadingIcon.StartAnimation();
        s_textBox.m_visible = true;
        s_textBox.SetNextText(L"Launching the game...");
        break;

    case Apploader::State::NO_DISC:
        s_loadingIcon.StopAnimation();
        s_textBox.m_visible = true;
        s_textBox.SetNextText(L"Please insert a disc.");
        break;

    case Apploader::State::READ_ERROR:
        s_loadingIcon.StopAnimation();
        s_textBox.m_visible = true;
        s_textBox.SetNextText(L"Unable the read the disc.");
        break;

    case Apploader::State::UNSUPPORTED_GAME:
        s_loadingIcon.StopAnimation();
        s_textBox.m_visible = true;
        s_textBox.SetNextText(L"This game is not supported by WiiLink WFC.");
        break;

    case Apploader::State::FATAL_ERROR:
        s_loadingIcon.StopAnimation();
        s_textBox.m_visible = true;
        s_textBox.SetNextText(L"An error has occurred");
        break;

    case Apploader::State::SHUTTING_DOWN:
        break;
    }

    s_loadingIcon.Calc();
    s_textBox.Calc();
    s_divider.Calc();
    s_logo.Calc();
}

static void LayoutDraw()
{
    s_divider.DrawBack();

    s_loadingIcon.Draw();
    s_textBox.Draw();
    s_logo.Draw();

    s_divider.DrawFront();
}

static void* ThreadFunc(void* arg)
{
    bool firstFrame = true;
    const GXColor bgColor = {0x00, 0x00, 0x00, 0x00};

    LayoutInit();

    // Main loop
    while (true) {
        if (Apploader::GetState() == Apploader::State::SHUTTING_DOWN) {
            s_divider.StartFadeOutBack();
            s_logo.SetAnimState(false);

            if (s_divider.IsFadeDone()) {
                break;
            }
        }

        if (s_shutdownType == Scene::ShutdownType::LAUNCH) {
            s_divider.StartFadeOut();
            s_logo.SetAnimState(false);

            if (s_divider.IsFadeDone()) {
                break;
            }
        }

        WPAD_ScanPads();

        // Check for HOME button press
        for (u32 i = 0; i < 4; i++) {
            if (s_shutdownType == Scene::ShutdownType::NONE &&
                WPAD_ButtonsDown(i) & WPAD_BUTTON_HOME) {
                std::printf("Exiting...\n");

                s_shutdownType = Scene::ShutdownType::EXIT;
                Apploader::ShutdownAsync();
            }
        }

        LayoutCalc();

        VIDEO_WaitVSync();

        GX_InvVtxCache();
        GX_InvalidateTexAll();

        {
            float mtx[4][4];
            auto rect = Scene::GetProjectionRect();
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

    if (s_shutdownType == Scene::ShutdownType::LAUNCH) {
        return nullptr;
    }

    Apploader::Shutdown();

    if (s_shutdownType == Scene::ShutdownType::POWER_OFF) {
        SYS_ResetSystem(SYS_POWEROFF, 0, 0);
    }

    std::exit(EXIT_SUCCESS);
    return nullptr;
}

static lwp_t s_thread;
static bool s_threadStarted = false;

void Scene::StartThread()
{
    LWP_CreateThread(&s_thread, ThreadFunc, nullptr, nullptr, 0x20000, 120);
    s_threadStarted = true;
}

void Scene::Shutdown(ShutdownType type)
{
    if (type != ShutdownType::NONE && s_shutdownType == ShutdownType::NONE) {
        s_shutdownType = type;
    }

    if (s_threadStarted) {
        LWP_JoinThread(s_thread, nullptr);
    }
}

void Scene::ShutdownAsync(ShutdownType type)
{
    if (type != ShutdownType::NONE && s_shutdownType == ShutdownType::NONE) {
        s_shutdownType = type;
    }
}
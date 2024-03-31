#include "Apploader.hpp"
#include "DI.hpp"
#include "Layout_Divider.hpp"
#include "Layout_LoadingIcon.hpp"
#include "Layout_TextBox.hpp"
#include "Scene.hpp"
#include "Util.hpp"
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ogc/cache.h>
#include <ogc/conf.h>
#include <ogc/consol.h>
#include <ogc/gx.h>
#include <ogc/system.h>
#include <ogc/video.h>
#include <wiiuse/wpad.h>

static GXRModeObj s_rmode = {};

#ifdef DEBUG_CONSOLE
static void* s_consoleXfb = nullptr;
#endif

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

#ifdef FORCE_PAL50
    dtv = false;
    pal60 = false;
    progressive = false;
    mode = VI_PAL;
#endif

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

int main(int argc, char** argv)
{
    (void) argc;
    (void) argv;

    LWP_SetThreadPriority(LWP_GetSelf(), 100);

    // Initialize video
    VIDEO_Init();
    s_rmode = GetRenderMode();
    VIDEO_Configure(&s_rmode);

    Scene::Init(&s_rmode);

#ifdef DEBUG_CONSOLE
    // Initialize LibOGC console
    s_consoleXfb = MEM_K0_TO_K1(SYS_AllocateFramebuffer(&s_rmode));
    CON_Init(
        s_consoleXfb, 20, 20, s_rmode.fbWidth, s_rmode.xfbHeight,
        s_rmode.fbWidth * VI_DISPLAY_PIX_SZ
    );
    VIDEO_SetNextFramebuffer(s_consoleXfb);
    VIDEO_SetBlack(false);
    VIDEO_Flush();
#endif

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
            Scene::ShutdownAsync(Scene::ShutdownType::EXIT);
            Apploader::ShutdownAsync();
        }

        if (event == STM_EVENT_POWER) {
            Scene::ShutdownAsync(Scene::ShutdownType::POWER_OFF);
            Apploader::ShutdownAsync();
        }
    });

    Apploader::StartThread();
    Scene::StartThread();

    LWP_SuspendThread(LWP_GetSelf());
}
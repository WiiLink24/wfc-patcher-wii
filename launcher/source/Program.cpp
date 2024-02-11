#include "Apploader.hpp"
#include "DI.hpp"
#include "Util.hpp"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ogc/conf.h>
#include <ogc/consol.h>
#include <ogc/gx.h>
#include <ogc/system.h>
#include <ogc/video.h>

static void* s_xfb[2] [[maybe_unused]] = {nullptr, nullptr};
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
    }

    return rmode;
}

int main(int argc, char** argv)
{
    (void) argc;
    (void) argv;

    // Set reserved area for our apploader
    SYS_SetArena1Hi(reinterpret_cast<void*>(0x80900000));

    // Initialize video
    VIDEO_Init();
    s_rmode = GetRenderMode();
    s_consoleXfb = MEM_K0_TO_K1(SYS_AllocateFramebuffer(&s_rmode));
    VIDEO_Configure(&s_rmode);

    // Initialize LibOGC console
    CON_Init(
        s_consoleXfb, 20, 20, s_rmode.fbWidth, s_rmode.xfbHeight,
        s_rmode.fbWidth * VI_DISPLAY_PIX_SZ
    );
    VIDEO_SetNextFramebuffer(s_consoleXfb);
    VIDEO_SetBlack(false);
    VIDEO_Flush();

    // Adjust to row 2 column 0
    std::printf("\x1b[%d;%dH", 2, 0);

    // Print logo
    std::printf("WiiLink WFC Launcher\n\n");

    // Initialize DI
    if (!DI::Init()) {
        std::printf("Failed to initialize DI\n");
        return EXIT_FAILURE;
    }

    std::printf("Reading the disc, please wait...\n");
    Apploader::LaunchDisc();
}
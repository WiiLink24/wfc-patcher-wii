#pragma once

#include <ogc/ipc.h>

namespace IOS
{

typedef ::ioctlv IOVector;

/**
 * Returns true if the program is running on Dolphin.
 */
bool IsDolphin();

enum {
    PATCH_IOSC_RSA_VERIFY = 1 << 0,
    PATCH_IOSC_KOREAN_KEY = 1 << 1,
    // PATCH_DI_UNENCRYPTED_READ = 1 << 2, // TODO
};

/**
 * Perform an IOS exploit and do the specified patches.
 */
void PatchIOS(u32 patchFlags);

/**
 * Perform an IOS exploit and do the specified patches asynchronously.
 */
void PatchIOSAsync(u32 patchFlags);

/**
 * Wait for the asynchronous patching to finish.
 */
void WaitForPatchIOS();

} // namespace IOS
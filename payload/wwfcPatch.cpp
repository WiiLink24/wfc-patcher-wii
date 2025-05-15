#include "wwfcPatch.hpp"
#include <import/revolution.h>
#include <wwfcCommon.h>

namespace wwfc::Patch
{

void ApplyPatch(u32 base, wwfc_patch& patch)
{
    if (patch.level & WWFC_PATCH_LEVEL_DISABLED || patch.address == 0) {
        // This can be done to disable patches that have already been applied
        return;
    }

    u32* address = reinterpret_cast<u32*>(patch.address);

    u32 baseArg0 = patch.arg0 & 0x80000000 ? patch.arg0 : base + patch.arg0;
    u32 baseArg1 = patch.arg1 & 0x80000000 ? patch.arg1 : base + patch.arg1;
    u32* arg0Ptr = reinterpret_cast<u32*>(baseArg0);
    u32* arg1Ptr = reinterpret_cast<u32*>(baseArg1);
    u32 flushSize = 0;

    switch (patch.type) {
    /**
     * Copy bytes specified in `args` to the destination `address`.
     * @param arg0 Pointer to the data to copy from.
     * @param arg1 Length of the data.
     */
    case WWFC_PATCH_TYPE_WRITE:
        std::memcpy(address, arg0Ptr, patch.arg1);
        flushSize = patch.arg1;
        break;

    /**
     * Write a branch: `address` = b `arg0`;
     * @param arg0 Branch destination address.
     * @param arg1 Not used.
     */
    case WWFC_PATCH_TYPE_BRANCH:
        *address = 0x48000000 | ((baseArg0 - patch.address) & 0x3FFFFFC);
        flushSize = sizeof(u32);
        break;

    /**
     * Write a branch with a branch back: `address` = b `arg0`; `arg1` = b
     * `address` + 4;
     * @param arg0 Branch destination address.
     * @param arg1 Address to write the branch back.
     */
    case WWFC_PATCH_TYPE_BRANCH_HOOK:
        *address = 0x48000000 | ((baseArg0 - patch.address) & 0x3FFFFFC);
        flushSize = sizeof(u32);

        *arg1Ptr = 0x48000000 | (((patch.address + 4) - baseArg1) & 0x3FFFFFC);
        RVL::DCFlushRange(arg1Ptr, sizeof(u32));
        RVL::ICInvalidateRange(arg1Ptr, sizeof(u32));
        break;

    /**
     * Write a branch with link: `address` = bl `arg0`
     * @param arg0 Branch destination address.
     * @param arg1 Not used.
     */
    case WWFC_PATCH_TYPE_CALL:
        *address = 0x48000001 | ((baseArg0 - patch.address) & 0x3FFFFFC);
        flushSize = sizeof(u32);
        break;

    /**
     * Write a branch using the count register:
     * `address` = \
     * lis `arg1`, `arg0`\@h; \
     * ori `arg1`, `arg1`, `arg0`\@l; \
     * mtctr `arg1`; \
     * bctr;
     * @param arg0 Branch destination address.
     * @param arg1 Temporary register to use for call.
     */
    case WWFC_PATCH_TYPE_BRANCH_CTR:
        if (patch.arg1 > 31) {
            return;
        }

        address[0] = 0x3C000000 | (baseArg0 >> 16) | (patch.arg1 << 21);
        address[1] = 0x60000000 | (baseArg0 & 0xFFFF) | (patch.arg1 << 21) |
                     (patch.arg1 << 16);
        address[2] = 0x7C0903A6 | (patch.arg1 << 21);
        address[3] = 0x4E800420;
        flushSize = sizeof(u32) * 4;
        break;

    /**
     * Write a branch with link using the count register:
     * `address` = \
     * lis `arg1`, `arg0`\@h; \
     * ori `arg1`, `arg1`, `arg0`\@l; \
     * mtctr `arg1`; \
     * bctrl;
     * @param arg0 Branch destination address.
     * @param arg1 Temporary register to use for call.
     */
    case WWFC_PATCH_TYPE_BRANCH_CTR_LINK:
        if (patch.arg1 > 31) {
            return;
        }

        address[0] = 0x3C000000 | (baseArg0 >> 16) | (patch.arg1 << 21);
        address[1] = 0x60000000 | (baseArg0 & 0xFFFF) | (patch.arg1 << 21) |
                     (patch.arg1 << 16);
        address[2] = 0x7C0903A6 | (patch.arg1 << 21);
        address[3] = 0x4E800421;
        flushSize = sizeof(u32) * 4;
        break;

    /**
     * Write a pointer specified in `arg0` to the destination `address`.
     * @param arg0 Pointer destination address.
     * @param arg1 Not used.
     */
    case WWFC_PATCH_TYPE_WRITE_POINTER:
        *address = baseArg0;
        flushSize = sizeof(u32);
        break;
    }

    RVL::DCFlushRange(address, flushSize);
    RVL::ICInvalidateRange(address, flushSize);

    // Disable the patch for completion
    patch.level |= WWFC_PATCH_LEVEL_DISABLED;
}

void ApplyPatchList(u32 base, wwfc_patch* patches, u32 patchCount)
{
    for (u32 i = 0; i < patchCount; i++) {
        ApplyPatch(base, patches[i]);
    }
}

} // namespace wwfc::Patch

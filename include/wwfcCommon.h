// wwfcCommon.h - WWFC common defs

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

typedef signed char s8;
typedef signed short s16;
typedef signed long s32;
typedef signed long long s64;

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned long u32;
typedef unsigned long long u64;

typedef float f32;
typedef double f64;

#ifndef SHA256_DIGEST_SIZE
#  define SHA256_DIGEST_SIZE 32
#endif

#define TITLE_TYPE_DISC 0
#define TITLE_TYPE_NAND 1

#ifndef WWFC_DOMAIN

#  ifdef PROD
#    define WWFC_DOMAIN "wiilink24.com"
#  else
#    define WWFC_DOMAIN "nwfc.wiinoma.com" // Points to localhost
#  endif

#endif

typedef struct {
    char magic[0xC]; // Always "WWFC/Payload"
    u32 total_size;
    u8 signature[0x100]; // RSA-2048 signature
} __attribute__((packed)) wwfc_payload_header;

typedef struct {
    u32 format_version; // Payload format version
    u32 format_version_compat; // Minimum payload format version that this
                               // payload is compatible with
    char name[0xC]; // Payload name (e.g. "RMCPD00")
    u32 version; // Payload version
    u32 got_start;
    u32 got_end;
    u32 fixup_start;
    u32 fixup_end;
    u32 patch_list_offset;
    u32 patch_list_end;
    u32 entry_point;
    u32 entry_point_no_got;
    u32 reserved[0x18 / 4];
    char build_timestamp[0x20];
} __attribute__((packed)) wwfc_payload_info;

typedef struct {
    wwfc_payload_header header;
    u8 salt[SHA256_DIGEST_SIZE];
    wwfc_payload_info info;
} __attribute__((packed)) wwfc_payload;

typedef enum {
    /**
     * Copy bytes specified in `args` to the destination `address`.
     * @param arg0 Pointer to the data to copy from.
     * @param arg1 Length of the data.
     */
    WWFC_PATCH_TYPE_WRITE = 0,

    /**
     * Write a branch: `address` = b `arg0`;
     * @param arg0 Branch destination address.
     * @param arg1 Not used.
     */
    WWFC_PATCH_TYPE_BRANCH = 1,

    /**
     * Write a branch with a branch back: `address` = b `arg0`; `arg1` = b
     * `address` + 4;
     * @param arg0 Branch destination address.
     * @param arg1 Address to write the branch back.
     */
    WWFC_PATCH_TYPE_BRANCH_HOOK = 2,

    /**
     * Write a branch with link: `address` = bl `arg0`
     * @param arg0 Branch destination address.
     * @param arg1 Not used.
     */
    WWFC_PATCH_TYPE_CALL = 3,

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
    WWFC_PATCH_TYPE_BRANCH_CTR = 4,

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
    WWFC_PATCH_TYPE_BRANCH_CTR_LINK = 5,
} wwfc_patch_type;

/**
 * Flags for different patch levels.
 */
typedef enum {

    /**
     * Critical, used for security patches and other things required to connect
     * to the server. This has no value and is always automatically applied.
     */
    WWFC_PATCH_LEVEL_CRITICAL = 0, // 0x00

    /**
     * Patches that fix bugs in the game, such as anti-freeze patches.
     */
    WWFC_PATCH_LEVEL_BUGFIX = 1 << 0, // 0x01

    /**
     * Patches required for parity with clients using a regular WWFC patcher.
     */
    WWFC_PATCH_LEVEL_PARITY = 1 << 1, // 0x02

    /**
     * Additional feature, not required to be compatible with regular clients.
     */
    WWFC_PATCH_LEVEL_FEATURE = 1 << 2, // 0x04

    /**
     * General support patches that may be redundant depending on the patcher.
     * Used in cases like URL patches.
     */
    WWFC_PATCH_LEVEL_SUPPORT = 1 << 3, // 0x08

    /**
     * Flag used to disable the patch if it has been already applied.
     */
    WWFC_PATCH_LEVEL_DISABLED = 1 << 4, // 0x10
} wwfc_patch_level;

typedef struct {
    u8 level; // wwfc_patch_level
    u8 type; // wwfc_patch_type
    u8 reserved[2];
    u32 address;

    union {
        u32 arg0;
        const void* arg0p;
        const u32* arg0p32;
    };

    u32 arg1;
} __attribute__((packed)) wwfc_patch;

#ifdef __cplusplus
}
#endif

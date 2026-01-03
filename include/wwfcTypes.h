//
// Common type definitions for WiiLink WFC
//

#ifndef WWFC_COMMON
#define WWFC_COMMON

#ifdef __cplusplus
extern "C" {
#endif

#define WWFC_SHA256_DIGEST_SIZE 32

#define WWFC_TITLE_TYPE_DISC 0
#define WWFC_TITLE_TYPE_NAND 1

#ifndef WWFC_DOMAIN
#  ifdef PROD
#    define WWFC_DOMAIN "wiilink24.com"
#  else
#    define WWFC_DOMAIN "nwfc.wiinoma.com" // Points to localhost
#  endif
#endif

#include "wwfcInteger.h"

typedef struct wwfc_payload_header wwfc_payload_header_t;
typedef struct wwfc_payload_header_ex wwfc_payload_header_ex_t;
typedef struct wwfc_payload_info wwfc_payload_info_t;
typedef struct wwfc_payload_info_ex wwfc_payload_info_ex_t;
typedef struct wwfc_payload wwfc_payload_t;
typedef struct wwfc_payload_ex wwfc_payload_ex_t;
typedef struct wwfc_patch wwfc_patch_t;

typedef wwfc_uint8_t wwfc_patch_type_t;
typedef wwfc_uint8_t wwfc_patch_level_t;
typedef wwfc_int32_t wwfc_function_t;
typedef wwfc_int32_t wwfc_key_t;
typedef wwfc_int32_t wwfc_boolean_t;

typedef wwfc_int32_t (*wwfc_payload_entry_t)(struct wwfc_payload*);
typedef wwfc_int32_t (*wwfc_function_exec_t)(wwfc_function_t function, ...);

struct wwfc_payload_header {
    wwfc_uint8_t magic[0xC]; // Always "WWFC/Payload"
    wwfc_uint32_t total_size;
    wwfc_uint8_t signature[0x100]; // RSA-2048 signature
};

struct wwfc_payload_header_ex {
    wwfc_uint8_t magic[0xC]; // Always "WWFC/Payload"
    void* end;
    wwfc_uint8_t signature[0x100]; // RSA-2048 signature
};

struct wwfc_payload_info {
    wwfc_uint32_t format_version; // Payload format version
    wwfc_uint32_t format_version_compat; // Minimum payload format version that
                                         // this payload is compatible with
    wwfc_uint8_t name[0xC]; // Payload name (e.g. "RMCPD00")
    wwfc_uint32_t version; // Payload version
    wwfc_uint32_t got_start;
    wwfc_uint32_t got_end;
    wwfc_uint32_t fixup_start;
    wwfc_uint32_t fixup_end;
    wwfc_uint32_t patch_list_offset;
    wwfc_uint32_t patch_list_end;
    wwfc_uint32_t entry_point;
    wwfc_uint32_t entry_point_no_got;
    wwfc_uint32_t function_exec;
    wwfc_uint32_t must_be_zero[0x14 / 4];
    wwfc_uint8_t build_timestamp[0x20];
};

struct wwfc_payload_info_ex {
    wwfc_uint32_t format_version; // Payload format version
    wwfc_uint32_t format_version_compat; // Minimum payload format version that
                                         // this payload is compatible with
    wwfc_uint8_t name[0xC]; // Payload name (e.g. "RMCPD00")
    wwfc_uint32_t version; // Payload version
    wwfc_uint32_t* got_start;
    wwfc_uint32_t* got_end;
    wwfc_uint32_t* fixup_start;
    wwfc_uint32_t* fixup_end;
    wwfc_patch_t* patch_list_offset;
    wwfc_patch_t* patch_list_end;
    wwfc_payload_entry_t entry_point;
    wwfc_payload_entry_t entry_point_no_got;
    wwfc_function_exec_t function_exec;
    wwfc_uint32_t must_be_zero[0x14 / 4];
    wwfc_uint8_t build_timestamp[0x20];
};

struct wwfc_payload {
    wwfc_payload_header_t header;
    wwfc_uint8_t salt[WWFC_SHA256_DIGEST_SIZE];
    wwfc_payload_info_t info;
};

struct wwfc_payload_ex {
    wwfc_payload_header_ex_t header;
    wwfc_uint8_t salt[WWFC_SHA256_DIGEST_SIZE];
    wwfc_payload_info_ex_t info;
} _;

enum wwfc_patch_type {
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

    /**
     * Write a pointer specified in `arg0` to the destination `address`.
     * @param arg0 Pointer destination address.
     * @param arg1 Not used.
     */
    WWFC_PATCH_TYPE_WRITE_POINTER = 6,
};

/**
 * Flags for different patch levels.
 */
enum wwfc_patch_level {
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
     * Patches required for parity with clients using a regular WiiLink WFC
     * patcher.
     */
    WWFC_PATCH_LEVEL_PARITY = 1 << 1, // 0x02

    /**
     * Additional feature, not required to be compatible with regular clients.
     */
    WWFC_PATCH_LEVEL_FEATURE = 1 << 2, // 0x04

    /**
     * General support in patching the game to connect to custom servers, that
     * may be redundant depending on the patcher. Used in cases like URL
     * patches.
     */
    WWFC_PATCH_LEVEL_SUPPORT = 1 << 3, // 0x08

    /**
     * Flag used to disable the patch if it has been already applied.
     */
    WWFC_PATCH_LEVEL_DISABLED = 1 << 4, // 0x10
};

struct wwfc_patch {
    wwfc_patch_level_t level;
    wwfc_patch_type_t type;
    wwfc_uint8_t reserved[2];
    wwfc_uint32_t address;
    wwfc_uint32_t arg0;
    wwfc_uint32_t arg1;
};

enum wwfc_function {
    /**
     * Apply a patch. Takes one parameter of const wwfc_patch*.
     */
    WWFC_FUNCTION_APPLY_PATCH = 0,

    /**
     * Get a value exported by the payload. Takes one parameter of wwfc_key and
     * returns the value as s32, or -1 if the key was not found.
     */
    WWFC_FUNCTION_GET_VALUE = 1,

    /**
     * Set a value exported by the payload. Takes parameters wwfc_key and s32
     * value. Returns 0 if successful, -1 otherwise.
     */
    WWFC_FUNCTION_SET_VALUE = 2,
};

enum wwfc_key {
    /**
     * wwfc_boolean to enable aggressive packet checks. Setting to FALSE
     * will allow players to send normally-invalid packets that may cause the
     * game to freeze. This currently only affects Mario Kart Wii. Defaults to
     * RESET.
     */
    WWFC_KEY_ENABLE_AGGRESSIVE_PACKET_CHECKS = 0,

    /**
     * wwfc_boolean to enable the EVENT item ID check in Mario Kart Wii.
     * WARNING: Setting this to FALSE may open up a security vulnerability!!
     * Please make sure invalid item IDs are handled by your custom code, and do
     * not touch this unless you know what you're doing!! Defaults to
     * TRUE.
     */
    WWFC_KEY_MKW_ENABLE_EVENT_ITEM_ID_CHECK = 1,

    /**
     * wwfc_boolean to enable the ultra shortcut patch in Mario Kart Wii. Note
     * the patch will still apply to the game's code even if this is set to
     * FALSE, but will be disabled. Defaults to RESET.
     */
    WWFC_KEY_MKW_ENABLE_ULTRA_UNCUT = 2,
};

enum wwfc_boolean {
    WWFC_BOOLEAN_FALSE = 0,
    WWFC_BOOLEAN_TRUE = 1,
    WWFC_BOOLEAN_RESET = 2,
};

#ifdef __cplusplus
}
#endif

#endif // WWFC_COMMON
#include "gct.h"
#include <wwfcError.h>

#ifndef WWFC_DOMAIN
#  define WWFC_DOMAIN "wiilink24.com"
#endif

// The stage 1 payload is downloaded from the server and verified using its MD5
// hash, preventing it from ever being updated, essentially making it an
// extension of this code. Security note: I'm aware that MD5 has been
// catastrophically broken in terms of collision attacks in the past, but from
// what I can tell of my research, it's still heavily resistant against
// pre-image and second pre-image attacks. It's generally not a good idea to
// rely on it anyway, but it's the only thing that's really universally shared
// across every game.

// Only patch once
GCT_IF_NOT_EQUAL_INST(ADDRESS_DWC_AUTH_ADD_MACADDR, b 0x7C)


#define HC(F, ...) F(ADDRESS_DWC_AUTH_ADD_CSNUM, AuthStage0Code, __VA_ARGS__)
#define HD(F, ...) F(ADDRESS_DWC_AUTH_ADD_MACADDR, AuthStage0Data, __VA_ARGS__)

// Skip adding the serial number to the auth request, and use the small area we
// cleared for code.
GCT_STRING(ADDRESS_DWC_AUTH_ADD_CSNUM, AuthStage0Code) // 0x800EE098

    // We have 0x800EE098 -> 0x800EE120
#if RPBJD00 || RPBJD01 || RPBJD02
    // Different for Pokemon Battle Revolution
    /* 0x80323EEC */ b       0xE8
#else
    /* 0x00 */ b       0x88
#endif
    // Hook at 0x800EE3AC (DWCi_Auth_HandleResponse, just after NHTTPGetBodyAll call)

#if ADDRESS_HBM_ALLOCATOR
    // Normal routine
    /* 0x04 */ HD(GCT_STRING_PTR, r31, LD_Stage1ParamBlock)
#else
#  if !ADDRESS_GH_ALLOC_FUNCTION
#    error Missing HBM allocator
#  endif
    // Guitar Hero alloc routine
    /* 0x04 */ HD(GCT_STRING_PTR_LWZU, r0, r31, LD_Stage1ParamBlock)
    /* 0x0C */ cmplwi  r0, 0
    /* 0x10 */ bne-    L_GHAllocDone

    /* 0x14 */ lis     r3, 0x20000@h
    /* 0x18 */ HC(GCT_STRING_BL_CALL, ADDRESS_GH_ALLOC_FUNCTION)
    /* 0x1C */ stw     r3, 0(r31)

L_GHAllocDone:
    // Adjust offsets from this point:
    // 0x0C -> 0x20
    // 0x58 -> 0x6C
#endif

#if ADDRESS_SSBB_GET_HEAP_FUNCTION
    // Super Smash Bros. Brawl doesn't have the HBM heap pointer set yet so we need to do it ourselves
    /* 0x0C */ li      r3, 6
    /* 0x10 */ HC(GCT_STRING_BL_CALL, ADDRESS_SSBB_GET_HEAP_FUNCTION)
    // TODO: This could be shortened to use r13
    /* 0x14 */ lis     r4, ADDRESS_HBM_ALLOCATOR@ha
    /* 0x18 */ stw     r3, ADDRESS_HBM_ALLOCATOR@l(r4)
    // Adjust offsets from this point:
    // 0x0C -> 0x1C
    // 0x58 -> 0x68
#endif

    // Calculate stage 1 MD5 hash
    /* 0x0C */ lwz     r3, 0xC(sp)
    /* 0x10 */ li      r4, STAGE1_SIZE + 0x2
    /* 0x14 */ addi    r5, sp, 0x1C
    /* 0x18 */ HC(GCT_STRING_BL_CALL, ADDRESS_MD5Digest)

    // Compare MD5 hash
    /* 0x1C */ addi    r3, sp, 0x1C
    /* 0x20 */ addi    r4, r31, LD_Stage1Hash - LD_Stage1ParamBlock
    /* 0x24 */ HC(GCT_STRING_BL_CALL, ADDRESS_strcmp)
    /* 0x28 */ cmpwi   r3, 0
    /* 0x2C */ beq-    L_HashMatched

    // Hash did not match
    /* 0x30 */ li      r0, WL_ERROR_PAYLOAD_STAGE0_HASH_MISMATCH
    /* 0x34 */ HC(GCT_STRING_B_CALL, ADDRESS_AUTH_HANDLERESP_ERROR) // 0x800EEAFC
    // ^^^ Not reached

L_HashMatched:
    // Hash matched, call stage 1
    /* 0x38 */ lwz     r3, 0xC(sp)
    /* 0x3C */ addi    r3, r3, 0x2
    /* 0x40 */ addi    r4, r31, LD_Stage1Param - LD_Stage1ParamBlock
            // lwz     r5, -0x68C4(r13)
    /* 0x44 */ LOAD_AUTH_REQ_INST_01
            // addi    r5, r5, 0x59E0
    /* 0x48 */ LOAD_AUTH_REQ_INST_02
    /* 0x4C */ mtctr   r3
    /* 0x50 */ bctrl // Call stage 1

    // Leave the auth function
    /* 0x54 */ HC(GCT_STRING_B_CALL, ADDRESS_AUTH_HANDLERESP_OUT) // 0x800EEB54
GCT_STRING_END(AuthStage0Code)


// Skip adding the MAC address to the auth request, and use the small area we
// cleared for data.
GCT_STRING(ADDRESS_DWC_AUTH_ADD_MACADDR, AuthStage0Data) // 0x800EDE84
    // We have 0x800EDE84 -> 0x800EDF00
    /* 0x00 */ b       0x7C
LD_Stage1Hash:
            // .string "6b66872c748546d7632a3d3626ec3cad"
    /* 0x04 */ .string STAGE1_HASH_MD5_STR
// LD_Stage1Hash end
    /* 0x25 -> 0x28 */ .balign 4
LD_Stage1Param:
LD_Stage1ParamBlock:
    /* 0x28 */ .long   0x00000000 // block
    /* 0x2C */ .long   ADDRESS_NHTTPCreateRequest // NHTTPCreateRequest (0x801D8FF8)
    /* 0x30 */ .long   ADDRESS_NHTTPSendRequestAsync // NHTTPSendRequestAsync (0x801D925C)
    /* 0x34 */ .long   ADDRESS_NHTTPDestroyResponse // NHTTPDestroyResponse (0x801D92F8)
    /* 0x38 */ .long   ADDRESS_HBM_ALLOCATOR // allocator (0x8028ADE4)
    /* 0x3C */ .long   ADDRESS_DWC_ERROR // dwc_error (0x802F1CB8)
            // .ascii  "RMCPD00\0\0"
    /* 0x40 */ .ascii  PAYLOAD
// LD_Stage1Param end
GCT_STRING_END(AuthStage0Data)


GCT_WRITE_BRANCH(ADDRESS_AUTH_HANDLERESP_HOOK, ADDRESS_DWC_AUTH_ADD_CSNUM + 0x4) // 0x800EE3A8, 0x800EE09C


// Skip DNS cache since the URLs aren't patched by this point
GCT_WRITE_BRANCH(ADDRESS_SKIP_DNS_CACHE, ADDRESS_SKIP_DNS_CACHE_CONTINUE) // 0x800D1518, 0x800D170C


GCT_ENDIF(1)


GCT_STRING(ADDRESS_AVAILABLE_URL + 0xD, AvailableURLOverride)
    .ascii  WWFC_DOMAIN "\0"
GCT_STRING_END(AvailableURLOverride)


GCT_STRING(ADDRESS_NASWII_AC_URL + 0x4, AuthURLOverride) // 0x8027A42C
    .ascii  "://naswii." WWFC_DOMAIN "/p0\0"
GCT_STRING_END(AuthURLOverride)



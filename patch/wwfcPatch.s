#include "gct.h"
#include <wwfcError.h>

// The stage 1 payload is downloaded from the server and verified using its MD5
// hash, preventing it from ever being updated, essentially making it an
// extension of this code. Security note: I'm aware that MD5 has been
// catastrophically broken in terms of collision attacks in the past, but from
// what I can tell of my research, it's still heavily resistant against
// pre-image and second pre-image attacks. It's generally not a good idea to
// rely on it anyway, but it's the only thing that's really universally shared
// across every game.

#ifndef WWFC_DOMAIN

#  ifdef PROD
#    define WWFC_DOMAIN "wiilink24.com"
#  else
#    define WWFC_DOMAIN "nwfc.wiinoma.com" // Points to localhost
#  endif

#endif

#define IBAT4U 560
#define IBAT4L 561
#define SRR0 26
#define SRR1 27

#ifndef NEEDS_IBAT_CONFIG

// Actually just blacklist known working games
#if RMCED00 | RMCJD00 | RMCKD00 | RMCPD00 | \
    RSBED01 | RSBED02 | RSBJD00 | RSBJD01 | RSBPD00 | RSBPD01 | \
    RTYPD00 | R4QED01 | R4QJD00 | R4QPD01 | R4QPD02 | WASJN0001
#  define NEEDS_IBAT_CONFIG 0
#else
#  define NEEDS_IBAT_CONFIG 1
#endif

#endif

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

#if R7EPD00 || R7EED00 || R7EJD00
    // NiGHTS: Journey of Dreams doesn't have the HBM heap at all times
    // These addresses are region-independent, much like other SEGA games
    /* 0x04 */ HD(GCT_STRING_PTR_LWZU, r0, r31, LD_Stage1ParamBlock)
    /* 0x0C */ cmplwi  r0, 0
    /* 0x10 */ bne-    L_AllocDone

    /* 0x14 */ lis     r3, 0x805BA418@h
    /* 0x18 */ ori     r3, r3, 0x805BA418@l
    /* 0x1C */ lis     r4, 0x20000@h
    /* 0x20 */ li      r5, 32
    /* 0x24 */ HC(GCT_STRING_BL_CALL, 0x80348F28)
    /* 0x28 */ stw     r3, 0(r31)

    // Adjust offsets from this point:
    // 0x0C -> 0x2C
    // 0x58 -> 0x78
#elif RMKED00 || RMKJD00 || RMKPD00
    // Mario Sports Mix allocates HBM at game start, but recreates the heap
    // everytime HBM is opened
    // Instead of allocating more memory, let's just truncate the HBM heap
    /* 0x04 */ lwz     r3, -0x6F98(r13) # HBM data (-0x10)
    /* 0x08 */ lis     r0, 0x60000@h
    /* 0x0C */ stw     r0, 0x10 + 0x2C(r3) # Heap size
    /* 0x10 */ lwz     r3, 0x10 + 0x10(r3) # Heap pointer
    /* 0x14 */ add     r3, r3, r0
    /* 0x18 */ HD(GCT_STRING_PTR_STWU, r3, r31, LD_Stage1ParamBlock)
    // Adjust offsets from this point:
    // 0x0C -> 0x20
    // 0x58 -> 0x6C
#elif R4QED01 | R4QJD00 | R4QKD00 | R4QPD01 | R4QPD02
    // Mario Strikers Charged works the same way as Mario Sports Mix
#if R4QKD00
    /* 0x04 */ lwz     r3, -0x1B90(r13) # HBM data (-0x4)
#elif R4QPD01 | R4QPD02
    /* 0x04 */ lwz     r3, -0x1B08(r13) # HBM data (-0x4)
#else
    /* 0x04 */ lwz     r3, -0x1AE8(r13) # HBM data (-0x4)
#endif
    /* 0x08 */ lis     r0, 0x60000@h
    /* 0x0C */ stw     r0, 0x4 + 0x2C(r3) # Heap size
    /* 0x10 */ lwz     r3, 0x4 + 0x10(r3) # Heap pointer
    /* 0x14 */ add     r3, r3, r0
    /* 0x18 */ HD(GCT_STRING_PTR_STWU, r3, r31, LD_Stage1ParamBlock)
    // Adjust offsets from this point:
    // 0x0C -> 0x20
    // 0x58 -> 0x6C
#elif ADDRESS_GH_ALLOC_FUNCTION
    // Guitar Hero games
    /* 0x04 */ HD(GCT_STRING_PTR_LWZU, r0, r31, LD_Stage1ParamBlock)
    /* 0x0C */ cmplwi  r0, 0
    /* 0x10 */ bne-    L_AllocDone

    /* 0x14 */ lis     r3, 0x20000@h
    /* 0x18 */ HC(GCT_STRING_BL_CALL, ADDRESS_GH_ALLOC_FUNCTION)
    /* 0x1C */ stw     r3, 0(r31)

    // Adjust offsets from this point:
    // 0x0C -> 0x20
    // 0x58 -> 0x6C
#elif ADDRESS_PES_ALLOC_FUNCTION
    // Pro Evolution Soccer games
    /* 0x04 */ HD(GCT_STRING_PTR_LWZU, r0, r31, LD_Stage1ParamBlock)
    /* 0x0C */ cmplwi  r0, 0
    /* 0x10 */ bne-    L_AllocDone

    /* 0x14 */ li      r3, 1
    /* 0x18 */ lis     r4, 0x20000@h
    /* 0x1C */ HC(GCT_STRING_BL_CALL, ADDRESS_PES_ALLOC_FUNCTION)
    /* 0x20 */ stw     r3, 0(r31)

    // Adjust offsets from this point:
    // 0x0C -> 0x24
    // 0x58 -> 0x70
#elif ADDRESS_BMST_RSO_LOCATOR
    /* 0x04 */ HD(GCT_STRING_PTR, r31, LD_Stage1ParamBlock)

    // Fortune Street uses an RSO for HBM, so we use this random pointer to try and find it
    // This is hacky but it saves on memory
    /* 0x0C */ lis     r3, ADDRESS_BMST_RSO_LOCATOR@ha
    /* 0x10 */ lwz     r3, ADDRESS_BMST_RSO_LOCATOR@l(r3)
    /* 0x14 */ addis   r3, r3, 0x2D49C@ha
    /* 0x18 */ addi    r3, r3, 0x2D49C@l
    /* 0x1C */ stw     r3, LD_Stage1ParamAllocator - LD_Stage1ParamBlock(r31)

    // Adjust offsets from this point:
    // 0x0C -> 0x20
    // 0x58 -> 0x6C
#else
    // Normal routine, executed if the HBM allocator is used
    /* 0x04 */ HD(GCT_STRING_PTR, r31, LD_Stage1ParamBlock)
#  if !ADDRESS_HBM_ALLOCATOR
#    error Missing HBM allocator
#  endif
#endif

#if ADDRESS_SSBB_GET_HEAP_FUNCTION
    // Super Smash Bros. Brawl doesn't have the HBM heap pointer set yet so we need to do it ourselves
    /* 0x0C */ li      r3, 5
    /* 0x10 */ HC(GCT_STRING_BL_CALL, ADDRESS_SSBB_GET_HEAP_FUNCTION)
    // TODO: This could be shortened to use r13
    /* 0x14 */ lis     r4, ADDRESS_HBM_ALLOCATOR@ha
    /* 0x18 */ stw     r3, ADDRESS_HBM_ALLOCATOR@l(r4)

    // Adjust offsets from this point:
    // 0x0C -> 0x1C
    // 0x58 -> 0x68
#endif

L_AllocDone:
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
#if NEEDS_IBAT_CONFIG
    /* 0x38 */ addi    r3, r31, L_ConfigMEM2IBAT - LD_Stage1ParamBlock
    /* 0x3C */ HC(GCT_STRING_BL_CALL, ADDRESS_RealMode) // 0x801A7C94

    // Adjust offsets from this point:
    // 0x38 -> 0x40
    // 0x58 -> 0x60
#endif

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

L_CustomAllocator:
#if WUNEN0005
    /* 0x58 */ .long   0x804F9D84
#elif WUNJN0002
    /* 0x58 */ .long   0x804FCF24
#elif WUNPN0002
    /* 0x58 */ .long   0x804FD224
#endif
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
LD_Stage1ParamAllocator:
#if WUNEN0005 || WUNJN0002 || WUNPN0002
    /* 0x38 */ .long   ADDRESS_DWC_AUTH_ADD_CSNUM + (L_CustomAllocator - AuthStage0Code) // allocator (custom)
#else
    /* 0x38 */ .long   ADDRESS_HBM_ALLOCATOR // allocator (0x8028ADE4)
#endif
    /* 0x3C */ .long   ADDRESS_DWC_ERROR // dwc_error (0x802F1CB8)
            // .ascii  "RMCPD00\0\0"
    /* 0x40 */ .ascii  PAYLOAD
// LD_Stage1Param end
    /* 0x4C */ .balign 4

#if NEEDS_IBAT_CONFIG
L_ConfigMEM2IBAT:
    /* 0x4C */ lis     r5, 0x1000
    /* 0x50 */ ori     r5, r5, 0x0002
    /* 0x54 */ lis     r4, 0x9000
    /* 0x58 */ ori     r4, r4, 0x1FFF

    /* 0x5C */ mtspr   IBAT4L, r5
    /* 0x60 */ mtspr   IBAT4U, r4
    /* 0x64 */ isync

    /* 0x68 */ ori     r3, r3, 0x30
    /* 0x6C */ mtspr   SRR1, r3
    /* 0x70 */ mflr    r3
    /* 0x74 */ mtspr   SRR0, r3
    /* 0x78 */ rfi
    // This is cutting it close, we can't add a single more instruction
#endif
GCT_STRING_END(AuthStage0Data)


GCT_WRITE_BRANCH(ADDRESS_AUTH_HANDLERESP_HOOK, ADDRESS_DWC_AUTH_ADD_CSNUM + 0x4) // 0x800EE3A8, 0x800EE09C


// Skip DNS cache since the URLs aren't patched by this point
GCT_WRITE_BRANCH(ADDRESS_SKIP_DNS_CACHE, ADDRESS_SKIP_DNS_CACHE_CONTINUE) // 0x800D1518, 0x800D170C


// Pokemon Battle Revolution and Mario Strikers Charged don't align the HTTP response buffer
// like how we need it to be
#if RPBED00 | RPBPD00 | R4QED01 | R4QJD00 | R4QPD01 | R4QPD02
GCT_WRITE_INSTR(ADDRESS_DWCi_Auth_SendRequest + 0x4C, addi r5, r6, 0x41C6)
GCT_WRITE_INSTR(ADDRESS_DWCi_Auth_SendRequest + 0x58, li r6, 0x1000 - 3)
#endif


GCT_ENDIF(1)


GCT_STRING(ADDRESS_AVAILABLE_URL + 0xD, AvailableURLOverride)
    .ascii  "gs." WWFC_DOMAIN "\0"
GCT_STRING_END(AvailableURLOverride)


GCT_STRING(ADDRESS_NASWII_AC_URL + 0x4, AuthURLOverride) // 0x8027A42C
    .ascii  "://nas." WWFC_DOMAIN "/w0\0"
GCT_STRING_END(AuthURLOverride)


// Undo some of the Wiimmfi patches to allow using a prepatched game
#if RMCED00 | RMCJD00 | RMCKD00 | RMCPD00

// Patch in DWCi_Auth_HandleResponse
GCT_WRITE_INSTR(ADDRESS_AUTH_HANDLERESP_HOOK - 0x8, cmpwi r3, 0)

// Patch in DWCi_Auth_InitInterface
#  if RMCED00
GCT_WRITE_INSTR(0x800ECA0C, mr r30, r3)
#  elif RMCJD00
GCT_WRITE_INSTR(0x800EC9CC, mr r30, r3)
#  elif RMCKD00
GCT_WRITE_INSTR(0x800ECB24, mr r30, r3)
#  elif RMCPD00
GCT_WRITE_INSTR(0x800ECAAC, mr r30, r3)
#  endif

#endif

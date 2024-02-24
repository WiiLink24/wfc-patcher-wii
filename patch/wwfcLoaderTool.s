#include "gct.h"
#include <wwfcError.h>

// Simple payload loader for developers. Loads a payload file from the NAND at
// /wwfc/<Title>, such as /wwfc/RMCPD00

// Hook at the beginning of DWCi_Auth_SendRequest and load the payload there
GCT_INSERT(ADDRESS_DWCi_Auth_SendRequest + 0x4, DebugLoadPayload)
    // Save registers as this is a difficult place to hook, cross-game
    // compatibility-wise
    mflr    r0
    lwz     r12, 0x0(r1)
    stw     r0, 0x4(r12)

    stw     r3, 0x48(r1)
    stw     r4, 0x4C(r1)
    stw     r5, 0x50(r1)
    stw     r6, 0x54(r1)
    stw     r7, 0x58(r1)
    stw     r8, 0x5C(r1)
    stw     r31, 0x60(r1)
    stw     r30, 0x64(r1)

    bl      L_BLTrick
L_BLTrick:
    mflr    r31

    // Check if the payload is already loaded
    lwz     r3, L_BlockAddr - L_BLTrick(r31)
    cmpwi   r3, 0
    beq     L_NotLoaded
    lwz     r4, 0x0(r3)
    xoris   r4, r4, 0x5757 // 'WW'
    cmplwi  r4, 0x4643 // 'FC'
    beq     L_HookEnd

L_NotLoaded:
    // Open the payload file
    addi    r3, r31, L_PayloadName - L_BLTrick
    li      r4, 1 // ISFS_OPEN_READ
    lis     r12, ADDRESS_IOS_Open@h
    ori     r12, r12, ADDRESS_IOS_Open@l
    mtctr   r12
    bctrl

    // Check for errors
    mr.     r30, r3
    li      r3, WL_ERROR_PAYLOAD_STAGE0_MISSING_STAGE1
    blt     L_HookNoSetError

    // Check if the block is already allocated
    lwz     r3, L_BlockAddr - L_BLTrick(r31)
    cmpwi   r3, 0
    bne     L_AllocDone

L_AllocBuffer:
    // Allocate memory for the payload. This part is just lazily copied from wwfcPatch.s
#if R7EPD00 || R7EED00 || R7EJD00
    // NiGHTS: Journey of Dreams doesn't have the HBM heap at all times
    // These addresses are region-independent, much like other SEGA games
    /* 0x14 */ lis     r3, 0x805BA418@h
    /* 0x18 */ ori     r3, r3, 0x805BA418@l
    /* 0x1C */ lis     r4, 0x20000@h
    /* 0x20 */ li      r5, 32
               lis     r12, 0x80348F28@h
               ori     r12, r12, 0x80348F28@l
               mtctr   r12
               bctrl
#elif RMKED00 || RMKJD00 || RMKPD00
    // Mario Sports Mix allocates HBM at game start, but recreates the heap
    // everytime HBM is opened
    // Instead of allocating more memory, let's just truncate the HBM heap
    /* 0x04 */ lwz     r3, -0x6F98(r13) # HBM data (-0x10)
    /* 0x08 */ lis     r0, 0x60000@h
    /* 0x0C */ stw     r0, 0x10 + 0x2C(r3) # Heap size
    /* 0x10 */ lwz     r3, 0x10 + 0x10(r3) # Heap pointer
    /* 0x14 */ add     r3, r3, r0
#elif R4QED01 | R4QJD00 | R4QKD00 | R4QPD01 | R4QPD02
    // Mario Strikers Charged works the same way as Mario Sports Mix
#if R4QKD00
    /* 0x04 */ lwz     r3, -0x1B90(r13) # HBM data (-0x4)
#else
    /* 0x04 */ lwz     r3, -0x1AE8(r13) # HBM data (-0x4)
#endif
    /* 0x08 */ lis     r0, 0x60000@h
    /* 0x0C */ stw     r0, 0x4 + 0x2C(r3) # Heap size
    /* 0x10 */ lwz     r3, 0x4 + 0x10(r3) # Heap pointer
    /* 0x14 */ add     r3, r3, r0
#elif ADDRESS_GH_ALLOC_FUNCTION
    // Guitar Hero games
    /* 0x14 */ lis     r3, 0x20000@h
               lis     r12, ADDRESS_GH_ALLOC_FUNCTION@h
               ori     r12, r12, ADDRESS_GH_ALLOC_FUNCTION@l
               mtctr   r12
               bctrl
#elif ADDRESS_PES_ALLOC_FUNCTION
    // Pro Evolution Soccer games
    /* 0x14 */ li      r3, 1
    /* 0x18 */ lis     r4, 0x20000@h
               lis     r12, ADDRESS_PES_ALLOC_FUNCTION@h
               ori     r12, r12, ADDRESS_PES_ALLOC_FUNCTION@l
               mtctr   r12
               bctrl
#else
    // Normal routine, executed if the HBM allocator is used
#if ADDRESS_SSBB_GET_HEAP_FUNCTION
    // Super Smash Bros. Brawl doesn't have the HBM heap pointer set yet so we need to do it ourselves
    li      r3, 5
    lis     r12, ADDRESS_SSBB_GET_HEAP_FUNCTION@h
    ori     r12, r12, ADDRESS_SSBB_GET_HEAP_FUNCTION@l
    mtctr   r12
    bctrl
#else
    lis     r3, ADDRESS_HBM_ALLOCATOR@ha
    lwz     r3, ADDRESS_HBM_ALLOCATOR@l(r3)
#endif

    // Alloc from HBM allocator
    lis     r4, 0x20000@h
    lwz     r12, 0(r3) // functions
    lwz     r12, 0(r12) // alloc
    mtctr   r12
    bctrl

#  if !ADDRESS_HBM_ALLOCATOR
#    error Missing HBM allocator
#  endif
#endif

L_AllocDone:
    // Read the payload file
    addi    r4, r3, 0x1F
    rlwinm. r4, r4, 0, ~0x1F // buffer
    stw     r4, L_BlockAddr - L_BLTrick(r31)
    li      r3, WL_ERROR_PAYLOAD_STAGE1_ALLOC
    beq     L_HookNoSetError
    mr      r3, r30 # fd
    lis     r5, 0x20000@h
    subi    r5, r5, 0x20 // size

    // Clear the buffer
    srwi    r0, r5, 5
    mtctr   r0
    li      r9, 0
L_ClearBuffer:
    dcbz    r9, r4
    addi    r9, r9, 0x20
    bdnz    L_ClearBuffer

    lis     r12, ADDRESS_IOS_Read@h
    ori     r12, r12, ADDRESS_IOS_Read@l
    mtctr   r12
    bctrl

    // Close the payload file
    mr      r3, r30
    lis     r12, ADDRESS_IOS_Close@h
    ori     r12, r12, ADDRESS_IOS_Close@l
    mtctr   r12
    bctrl

    lwz     r3, L_BlockAddr - L_BLTrick(r31)
    lwz     r4, 0x0(r3)
    xoris   r4, r4, 0x5757 // 'WW'
    cmplwi  r4, 0x4643 // 'FC'
    bne     L_HookError

    // Call the payload
    lwz     r12, 0x160(r3)
    add     r12, r12, r3
    mtctr   r12
    bctrl
    cmpwi   r3, 0
    bne     L_HookNoSetError
    b       L_HookEnd

L_BlockAddr:
    .long   0x0

L_PayloadName:
    .ascii  "/wwfc/" PAYLOAD "\0"

    .balign 4
L_HookError:
    li      r3, WL_ERROR_PAYLOAD_STAGE1_HEADER_CHECK
    // Fall through
L_HookNoSetError:
    lis     r4, ADDRESS_DWC_ERROR@ha
    stw     r3, ADDRESS_DWC_ERROR@l(r4)

    lwz     r31, 0x60(r1)
    lwz     r30, 0x64(r1)

    lwz     r1, 0x0(r1)
    lwz     r0, 0x4(r1)
    mtlr    r0
    blr

L_HookEnd:
    // Reload registers
    lwz     r3, 0x48(r1)
    lwz     r4, 0x4C(r1)
    lwz     r5, 0x50(r1)
    lwz     r6, 0x54(r1)
    lwz     r7, 0x58(r1)
    lwz     r8, 0x5C(r1)
    lwz     r31, 0x60(r1)
    lwz     r30, 0x64(r1)

    lwz     r12, 0x0(r1)
    lwz     r0, 0x4(r12)
GCT_INSERT_END(DebugLoadPayload)


// Skip DNS cache since the URLs aren't patched by this point
GCT_WRITE_BRANCH(ADDRESS_SKIP_DNS_CACHE, ADDRESS_SKIP_DNS_CACHE_CONTINUE) // 0x800D1518, 0x800D170C


GCT_STRING(ADDRESS_AVAILABLE_URL + 0xD, AvailableURLOverride)
    .ascii  "gs.wiilink24.com\0"
GCT_STRING_END(AvailableURLOverride)
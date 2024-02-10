#include "ASM.h"

#define DOL_SECTION_COUNT (7 + 11)

#define DOL_SECTION 0x0
#define DOL_SECTION_ADDR 0x48
#define DOL_SECTION_SIZE 0x90

#define DOL_BSS_ADDRESS 0xD8
#define DOL_BSS_SIZE 0xDC
#define DOL_ENTRY_POINT 0xE0

ASM_SYMBOL_START(RunDOL, .text)
    // Clear BSS
    lwz     r4, DOL_BSS_ADDRESS(r3)
    lwz     r5, DOL_BSS_SIZE(r3)

    srwi    r0, r5, 5
    mtctr   r0

L_BSSLoop:
    dcbz    0, r4
    dcbf    0, r4
    addi    r4, r4, 32
    bdnz+   L_BSSLoop

    li      r4, 0 // Current section

L_SectionLoop:
    slwi    r0, r4, 2

    addi    r7, r3, DOL_SECTION_SIZE
    lwzx    r7, r7, r0 // Copy size

    cmpwi   r7, 0
    beq-    L_SectionLoopEnd

    lwzx    r5, r3, r0
    add     r5, r3, r5 // Copy src
    addi    r6, r3, DOL_SECTION_ADDR
    lwzx    r6, r6, r0 // Copy dest

    srwi    r0, r7, 5
    mtctr   r0

L_CopyLoop:
    dcbz    0, r6
#if 1
    lwz     r0, 0x00(r5)
    stw     r0, 0x00(r6)
    lwz     r0, 0x04(r5)
    stw     r0, 0x04(r6)
    lwz     r0, 0x08(r5)
    stw     r0, 0x08(r6)
    lwz     r0, 0x0C(r5)
    stw     r0, 0x0C(r6)
    lwz     r0, 0x10(r5)
    stw     r0, 0x10(r6)
    lwz     r0, 0x14(r5)
    stw     r0, 0x14(r6)
    lwz     r0, 0x18(r5)
    stw     r0, 0x18(r6)
    lwz     r0, 0x1C(r5)
    stw     r0, 0x1C(r6)
#else
    lmw     r24, 0x00(r5)
    stmw    r24, 0x00(r6)
#endif
    dcbf    0, r6
    icbi    0, r6
    addi    r5, r5, 0x20
    addi    r6, r6, 0x20
    bdnz+   L_CopyLoop

L_SectionLoopEnd:
    addi    r4, r4, 1
    cmpwi   r4, DOL_SECTION_COUNT
    blt+    L_SectionLoop

    lwz     r4, DOL_ENTRY_POINT(r3)
    mtctr   r4
    bctrl

L_InfiniteLoop:
    b       L_InfiniteLoop
ASM_SYMBOL_END(RunDOL)

ASM_SYMBOL_START(RunDOLEnd, .text)
ASM_SYMBOL_END(RunDOLEnd)
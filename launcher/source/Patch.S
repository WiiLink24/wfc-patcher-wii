#include "ASM.h"

ASM_SYMBOL_START(g_wwfcPatchStart, .text)
    lis     r12, g_wwfcPatchStage1Data@h
    ori     r12, r12, g_wwfcPatchStage1Data@l

    lbz     r11, L_Stage1DataFinished - g_wwfcPatchStage1Data(r12)
    cmpwi   r11, 0
    bne     g_wwfcPatchAuthExit

    lwz     r11, L_Stage1DataDWCError - g_wwfcPatchStage1Data(r12)
    lwz     r11, 0(r11)
    cmpwi   r11, -1 // Specifically the stage1 retry error
    beq     L_SetFinishedAndExit

    lwz     r3, L_Stage1DataStage1Addr - g_wwfcPatchStage1Data(r12)
    mr      r4, r12

    .global g_wwfcPatchLoadAuthWorkReq
g_wwfcPatchLoadAuthWorkReq:
    nop // Placeholder load auth work req
    nop // Placeholder load auth work req
    nop // Placeholder load auth work req
    
    mtctr   r3
    bctr

L_SetFinishedAndExit:
    lis     r11, 1
    stw     r11, L_Stage1DataFinished - g_wwfcPatchStage1Data(r12)
    // Fall through

    .global g_wwfcPatchAuthExit
g_wwfcPatchAuthExit:
    nop // Placeholder original instruction
    b       -4 // Placeholder branch back

    .global g_wwfcPatchStage1Data
g_wwfcPatchStage1Data:
    .long   0x00000000 // block
    .long   0x00000000 // NHTTPCreateRequest (0x801D8FF8)
    .long   0x00000000 // NHTTPSendRequestAsync (0x801D925C)
    .long   0x00000000 // NHTTPDestroyResponse (0x801D92F8)
    .long   0x00000000 // allocator (0x8028ADE4)
L_Stage1DataDWCError:
    .long   0x00000000 // dwc_error (0x802F1CB8)
L_Stage1DataPayloadName:
    .space  9

L_Stage1DataFinished:
    .byte   0x00 // finished
    .balign 4
L_Stage1DataStage1Addr:
    .long   0x00000000 // stage1 address

    .global g_wwfcPatchEnd
g_wwfcPatchEnd:
ASM_SYMBOL_END(g_wwfcPatchStart)


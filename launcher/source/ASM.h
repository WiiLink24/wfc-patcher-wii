#pragma once

#define cr0 0
#define cr1 1
#define cr2 2
#define cr3 3
#define cr4 4
#define cr5 5
#define cr6 6
#define cr7 7

#define r0 %r0
#define r1 %r1
#define sp %r1
#define r2 %r2
#define toc %r2
#define r3 %r3
#define r4 %r4
#define r5 %r5
#define r6 %r6
#define r7 %r7
#define r8 %r8
#define r9 %r9
#define r10 %r10
#define r11 %r11
#define r12 %r12
#define r13 %r13
#define r14 %r14
#define r15 %r15
#define r16 %r16
#define r17 %r17
#define r18 %r18
#define r19 %r19
#define r20 %r20
#define r21 %r21
#define r22 %r22
#define r23 %r23
#define r24 %r24
#define r25 %r25
#define r26 %r26
#define r27 %r27
#define r28 %r28
#define r29 %r29
#define r30 %r30
#define r31 %r31

#define f0 0
#define f1 1
#define f2 2
#define f3 3
#define f4 4
#define f5 5
#define f6 6
#define f7 7
#define f8 8
#define f9 9
#define f10 10
#define f11 11
#define f12 12
#define f13 13
#define f14 14
#define f15 15
#define f16 16
#define f17 17
#define f18 18
#define f19 19
#define f20 20
#define f21 21
#define f22 22
#define f23 23
#define f24 24
#define f25 25
#define f26 26
#define f27 27
#define f28 28
#define f29 29
#define f30 30
#define f31 31

#define ASM_FUNCTION_START(_NAME, _SECT) \
    .text; \
    .section _SECT##.##_NAME, "ax"; \
    .global _NAME; \
    .balign 4; \
_NAME:


#define ASM_FUNCTION_END(_NAME) \
    .size   _NAME, . - _NAME

#define ASM_SYMBOL_START(_NAME, _SECT) \
    .section _SECT##.##_NAME, "a"; \
    .global _NAME; \
    .balign 16; \
_NAME:


#define ASM_SYMBOL_END(_NAME) \
    .size   _NAME, . - _NAME

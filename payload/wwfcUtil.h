#pragma once

#include <wwfcCommon.h>

#define SECTION(_SECTION) __attribute__((__section__(_SECTION)))
#define LONGCALL __attribute__((__longcall__))

#define FILL(_START, _END) u8 _##_START[_END - _START]

#define _STRIFY1(_VAL) #_VAL
#define STRIFY(_VAL) _STRIFY1(_VAL)

#define AT(_ADDRESS) asm(STRIFY(_ADDRESS))

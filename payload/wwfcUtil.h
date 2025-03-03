#pragma once

#include <wwfcCommon.h>

#define SECTION(_SECTION) __attribute__((__section__(_SECTION)))
#define LONGCALL __attribute__((__longcall__))
#define PACKED __attribute__((__packed__))

#define FILL(_START, _END) u8 _##_START[_END - _START]

#define _STRIFY1(_VAL) #_VAL
#define STRIFY(_VAL) _STRIFY1(_VAL)

#define AT(_ADDRESS) asm(STRIFY(_ADDRESS))

// Mario Kart Wii port macro

#if RMCPD00

// Mario Kart Wii Rev 0 [PAL]
#  define RMC 1

#  define RMCXD_PORT(P, E, J, K) P
#  define RMCXN_PORT(P, E, J, K)

#elif RMCED00

// Mario Kart Wii Rev 0 [NTSC-U]
#  define RMC 1

#  define RMCXD_PORT(P, E, J, K) E
#  define RMCXN_PORT(P, E, J, K)

#elif RMCJD00

// Mario Kart Wii Rev 0 [NTSC-J]
#  define RMC 1

#  define RMCXD_PORT(P, E, J, K) J
#  define RMCXN_PORT(P, E, J, K)

#elif RMCKD00

// Mario Kart Wii Rev 0 [NTSC-J]
#  define RMC 1

#  define RMCXD_PORT(P, E, J, K) K
#  define RMCXN_PORT(P, E, J, K)

#elif RMCPN0001

// Mario Kart Channel Rev 0 [PAL]
#  define RMCN 1

#  define RMCXD_PORT(P, E, J, K)
#  define RMCXN_PORT(P, E, J, K) P

#elif RMCEN0001

// Mario Kart Channel Rev 0 [NTSC-U]
#  define RMCN 1

#  define RMCXD_PORT(P, E, J, K)
#  define RMCXN_PORT(P, E, J, K) E

#elif RMCJN0001

// Mario Kart Channel Rev 0 [NTSC-J]
#  define RMCN 1

#  define RMCXD_PORT(P, E, J, K)
#  define RMCXN_PORT(P, E, J, K) J

#elif RMCKN0001

// Mario Kart Channel Rev 0 [NTSC-K]
#  define RMCN 1

#  define RMCXD_PORT(P, E, J, K)
#  define RMCXN_PORT(P, E, J, K) K

#endif

#if RMC || RMCN

#  define RMCX_PORT(P, E, J, K, NP, NE, NJ, NK)                                \
      RMCXD_PORT(P, E, J, K) RMCXN_PORT(NP, NE, NJ, NK)

#endif

// Super Smash Bros. Brawl port macro

#if RSBED01 | RSBED02

// Super Smash Bros. Brawl Rev 1, Rev 2 [NTSC-U]
#define RSB 1
#define RSBX_PORT(E, J, P0, P1) E

#elif RSBJD00 | RSBJD01

// Super Smash Bros. Brawl Rev 0, Rev 1 [NTSC-J]
#define RSB 1
#define RSBX_PORT(E, J, P0, P1) J

#elif RSBPD00

// Super Smash Bros. Brawl Rev 0 [PAL]
#define RSB 1
#define RSBX_PORT(E, J, P0, P1) P0

#elif RSBPD01

// Super Smash Bros. Brawl Rev 1 [PAL]
#define RSB 1
#define RSBX_PORT(E, J, P0, P1) P1

#endif

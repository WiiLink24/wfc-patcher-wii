#pragma once

#include <wwfcCommon.h>

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

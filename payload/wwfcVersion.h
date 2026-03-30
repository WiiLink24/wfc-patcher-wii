#pragma once

#ifndef WWFC_PAYLOAD_MAJOR
#  define WWFC_PAYLOAD_MAJOR 0
#  define WWFC_PAYLOAD_MINOR 2
#  define WWFC_PAYLOAD_BETA 2
#endif

static_assert(
    WWFC_PAYLOAD_MAJOR < 256 && WWFC_PAYLOAD_MINOR < 4096 &&
        WWFC_PAYLOAD_BETA < 4096,
    "Payload version numbers are too large"
);

#define WWFC_VERSION_STRING_HELPER(_MAJOR, _MINOR, _BETA)                      \
    #_MAJOR "." #_MINOR "." #_BETA

#define WWFC_VERSION_STRING                                                    \
    WWFC_VERSION_STRING_HELPER(                                                \
        WWFC_PAYLOAD_MAJOR, WWFC_PAYLOAD_MINOR, WWFC_PAYLOAD_BETA              \
    )

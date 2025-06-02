#pragma once

#ifndef WWFC_PAYLOAD_MAJOR
#  define WWFC_PAYLOAD_MAJOR 0
#  define WWFC_PAYLOAD_MINOR 0
#  define WWFC_PAYLOAD_BETA 5
#endif

static_assert(
    WWFC_PAYLOAD_MAJOR < 256 && WWFC_PAYLOAD_MINOR < 4096 &&
        WWFC_PAYLOAD_BETA < 4096,
    "Payload version numbers are too large"
);
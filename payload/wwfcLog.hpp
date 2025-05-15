#pragma once

#include "import/revolution.h"

namespace wwfc::Log
{

#define WWFC_LOG_INFO(_STRING)                                                 \
    RVL::OSReport("%s" _STRING "\n", "WWFC_INFO    : ")
#define WWFC_LOG_INFO_FMT(_FORMAT, ...)                                        \
    RVL::OSReport("%s" _FORMAT "\n", "WWFC_INFO    : ", __VA_ARGS__)

#define WWFC_LOG_WARN(_STRING)                                                 \
    RVL::OSReport("%s" _STRING "\n", "WWFC_WARN    : ")
#define WWFC_LOG_WARN_FMT(_FORMAT, ...)                                        \
    RVL::OSReport("%s" _FORMAT "\n", "WWFC_WARN    : ", __VA_ARGS__)

#define WWFC_LOG_ERROR(_STRING)                                                \
    RVL::OSReport("%s" _STRING "\n", "++WWFC_ERROR : ")
#define WWFC_LOG_ERROR_FMT(_FORMAT, ...)                                       \
    RVL::OSReport("%s" _FORMAT "\n", "++WWFC_ERROR : ", __VA_ARGS__)

#define WWFC_LOG_NOTICE(_STRING)                                               \
    RVL::OSReport("%s" _STRING "\n", "WWFC_NOTICE  : ")
#define WWFC_LOG_NOTICE_FMT(_FORMAT, ...)                                      \
    RVL::OSReport("%s" _FORMAT "\n", "WWFC_NOTICE  : ", __VA_ARGS__)

} // namespace wwfc::Log

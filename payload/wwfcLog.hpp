#pragma once

#include "import/revolution.h"

namespace wwfc::Log
{

#define LOG_INFO(_STRING) RVL::OSReport("%s" _STRING "\n", "WWFC_INFO    : ")
#define LOG_INFO_FMT(_FORMAT, ...)                                             \
    RVL::OSReport("%s" _FORMAT "\n", "WWFC_INFO    : ", __VA_ARGS__)

#define LOG_WARN(_STRING) RVL::OSReport("%s" _STRING "\n", "WWFC_WARN    : ")
#define LOG_WARN_FMT(_FORMAT, ...)                                             \
    RVL::OSReport("%s" _FORMAT "\n", "WWFC_WARN    : ", __VA_ARGS__)

#define LOG_ERROR(_STRING) RVL::OSReport("%s" _STRING "\n", "++WWFC_ERROR : ")
#define LOG_ERROR_FMT(_FORMAT, ...)                                            \
    RVL::OSReport("%s" _FORMAT "\n", "++WWFC_ERROR : ", __VA_ARGS__)

#define LOG_NOTICE(_STRING) RVL::OSReport("%s" _STRING "\n", "WWFC_NOTICE  : ")
#define LOG_NOTICE_FMT(_FORMAT, ...)                                           \
    RVL::OSReport("%s" _FORMAT "\n", "WWFC_NOTICE  : ", __VA_ARGS__)

} // namespace wwfc::Log

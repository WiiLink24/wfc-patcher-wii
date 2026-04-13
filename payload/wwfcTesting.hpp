#pragma once

#include "wwfcLibC.hpp"
#include "wwfcLog.hpp"

#define WWFC_TESTING

namespace wwfc
{

struct Testing {

#if defined(WWFC_TESTING)

    template <std::size_t N>
    struct FileName {
        constexpr FileName(const char (&str)[N])
          : value{}
        {
            for (std::size_t i = 0; i < N; i++) {
                value[i] = str[i];
            }
        }

        char value[N];
    };

    template <FileName Name = "">
    struct Method {
        using Test = int (*)(const Testing&);

        constexpr Method(auto function, short line = __builtin_LINE())
          : function(+function)
          , line(line)
          , file_length(sizeof Name.value)
          , file{}
        {
            for (std::size_t i = 0; i < sizeof Name.value; i++) {
                file[i] = Name.value[i];
            }
        }

        Method(const Method&) = delete;

        Test function;
        short line;
        char file_length;
        char file[sizeof Name.value];
    };

#else

    template <std::size_t>
    struct Method {
        using Test = int (*)(const Testing&);

        constexpr Method(auto function)
            requires(std::is_same_v<decltype(+function), Test>)
        {
        }
    };

#endif

    constexpr int Error(int line = __builtin_LINE()) const
    {
        return line;
    }

    void RunTests() const
    {
#if defined(WWFC_TESTING)
        WWFC_LOG_INFO("Running tests...");

        extern const Method<> _G_WWFCTestingStart AT(_G_WWFCTestingStart);
        extern const Method<> _G_WWFCTestingEnd AT(_G_WWFCTestingEnd);
        int errorCount = 0;
        int testCount = 0;
        std::size_t next;
        for (const Method<>* method = &_G_WWFCTestingStart;
             method < &_G_WWFCTestingEnd;
             // Align to the next method
             next = reinterpret_cast<std::size_t>(method + 1) +
                    method->file_length,
                             next = (next + alignof(Method<>) - 1) &
                                    ~(alignof(Method<>) - 1),
                             method = reinterpret_cast<const Method<>*>(next)) {
            testCount++;
            WWFC_LOG_INFO_FMT(
                "Running test at %s:%d...", method->file, method->line
            );
            int result = method->function(*this);
            if (result == 0) {
                continue;
            }
            errorCount++;
            WWFC_LOG_ERROR_FMT(
                "Test at %s:%d failed at line %d", method->file, method->line,
                result
            );
        }

        if (errorCount == 0) {
            WWFC_LOG_NOTICE_FMT("All %d test(s) passed!", testCount);
        } else {
            WWFC_LOG_ERROR_FMT(
                "%d test(s) of %d failed!", errorCount, testCount
            );
        }
#endif
    }

#define METHOD_CONCAT_IMPL(a, b) a##b
#define METHOD_CONCAT(a, b) METHOD_CONCAT_IMPL(a, b)

#define WWFC_DEFINE_TEST                                                       \
    [[__gnu__::__section__(                                                    \
        "wwfc_testing"                                                         \
    )]] constinit ::wwfc::Testing::Method<__FILE_NAME__>                       \
    METHOD_CONCAT(_wwfc_test_method_, __COUNTER__)
};

} // namespace wwfc
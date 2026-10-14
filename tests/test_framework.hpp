#pragma once

// Minimal self-contained unit-test framework (the project must not depend on
// external libraries, so no GoogleTest/Catch2). Each test file defines TEST_CASE
// bodies and ends with TEST_MAIN().

#include <cstdio>
#include <vector>

namespace testing {

struct TestCase {
    const char* name;
    void (*run)();
};

inline std::vector<TestCase>& registry() {
    static std::vector<TestCase> cases;
    return cases;
}

inline int& check_count() {
    static int count = 0;
    return count;
}

inline int& failure_count() {
    static int count = 0;
    return count;
}

struct Registrar {
    Registrar(const char* name, void (*run)()) {
        registry().push_back({name, run});
    }
};

inline void report_failure(const char* file, int line, const char* expression) {
    ++failure_count();
    std::fprintf(stderr, "  FAIL %s:%d: %s\n", file, line, expression);
}

inline int run_all() {
    std::fprintf(stderr, "[test] running %zu test cases\n", registry().size());
    for (const auto& test : registry()) {
        const int failures_before = failure_count();
        test.run();
        std::fprintf(stderr, "[test] %-45s %s\n", test.name,
                     failure_count() == failures_before ? "ok" : "FAILED");
    }
    std::fprintf(stderr, "[test] %d checks, %d failures\n", check_count(), failure_count());
    return failure_count() == 0 ? 0 : 1;
}

} // namespace testing

#define TEST_CASE(name)                                                        \
    static void name();                                                        \
    static ::testing::Registrar registrar_##name(#name, &name);                \
    static void name()

#define CHECK(condition)                                                       \
    do {                                                                       \
        ++::testing::check_count();                                            \
        if (!(condition)) {                                                    \
            ::testing::report_failure(__FILE__, __LINE__, #condition);         \
        }                                                                      \
    } while (0)

#define CHECK_EQ(expected, actual)                                             \
    do {                                                                       \
        ++::testing::check_count();                                            \
        const auto& check_expected = (expected);                               \
        const auto& check_actual = (actual);                                   \
        if (!(check_expected == check_actual)) {                               \
            ::testing::report_failure(__FILE__, __LINE__,                      \
                                      #expected " == " #actual);               \
        }                                                                      \
    } while (0)

#define TEST_MAIN()                                                            \
    int main() { return ::testing::run_all(); }

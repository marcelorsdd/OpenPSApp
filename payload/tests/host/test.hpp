#pragma once

#include <cstdio>
#include <functional>
#include <string>
#include <type_traits>
#include <vector>

namespace test {
struct Case {
    const char *name;
    std::function<void()> fn;
};

inline std::vector<Case> &Registry() {
    static std::vector<Case> cases;
    return cases;
}

inline int &Failures() {
    static int failures = 0;
    return failures;
}

struct Register {
    Register(const char *name, std::function<void()> fn) { Registry().push_back({name, std::move(fn)}); }
};

inline std::string Show(const std::string &v) { return "\"" + v + "\""; }
inline std::string Show(const char *v) { return Show(std::string(v)); }
inline std::string Show(bool v) { return v ? "true" : "false"; }
template <typename T, typename = std::enable_if_t<std::is_arithmetic_v<T>>>
std::string Show(T v) {
    return std::to_string(v);
}

}

#define TEST(name)                                                 \
    static void test_##name();                                     \
    static test::Register register_##name(#name, test_##name);     \
    static void test_##name()

#define CHECK(cond)                                                                \
    do {                                                                           \
        if (!(cond)) {                                                             \
            std::printf("  FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond);          \
            ++test::Failures();                                                    \
        }                                                                          \
    } while (0)

#define CHECK_EQ(actual, expected)                                                                     \
    do {                                                                                               \
        auto &&a_ = (actual);                                                                          \
        auto &&e_ = (expected);                                                                        \
        if (!(a_ == e_)) {                                                                             \
            std::printf("  FAIL %s:%d: %s == %s\n    got:  %s\n    want: %s\n", __FILE__, __LINE__,      \
                        #actual, #expected, test::Show(a_).c_str(), test::Show(e_).c_str());           \
            ++test::Failures();                                                                        \
        }                                                                                              \
    } while (0)

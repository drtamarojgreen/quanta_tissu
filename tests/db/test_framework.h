#ifndef TEST_FRAMEWORK_H
#define TEST_FRAMEWORK_H

#include <iostream>
#include <vector>
#include <string>
#include <functional>
#include <stdexcept>
#include <sstream> // For FAIL macro

// Simple testing framework
namespace TestFramework {

struct TestCase {
    std::string name;
    std::function<void()> func;
};

inline std::vector<TestCase>& get_test_cases() {
    static std::vector<TestCase> test_cases;
    return test_cases;
}

inline void register_test(const std::string& name, std::function<void()> func) {
    get_test_cases().push_back({name, func});
}

#define TEST_CASE(name) \
    void test_##name(); \
    struct Register##name { \
        Register##name() { TestFramework::register_test(#name, test_##name); } \
    } register_##name; \
    void test_##name()

#define ASSERT_TRUE(condition) \
    if (!(condition)) { \
        std::cerr << "Assertion failed: " << #condition << " at " << __FILE__ << ":" << __LINE__ << std::endl; \
        throw std::runtime_error("Assertion failed"); \
    }

#define ASSERT_FALSE(condition) ASSERT_TRUE(!(condition))
#define ASSERT_EQ(expected, actual) ASSERT_TRUE((expected) == (actual))
#define ASSERT_NE(expected, actual) ASSERT_TRUE((expected) != (actual))
#define ASSERT_THROW(expression, exception_type) \
    try { \
        expression; \
        ASSERT_TRUE(false && "Expected exception not thrown"); \
    } catch (const exception_type& e) { \
        ASSERT_TRUE(true); \
    } catch (...) { \
        ASSERT_TRUE(false && "Caught unexpected exception type"); \
    }

#define FAIL(msg) \
    do { \
        std::stringstream ss; \
        ss << "Failure at " << __FILE__ << ":" << __LINE__ << ": " << msg; \
        throw std::runtime_error(ss.str()); \
    } while (false)


inline void run_all_tests() {
    int passed = 0;
    int failed = 0;
    for (const auto& test_case : get_test_cases()) {
        std::cout << "Running test: " << test_case.name << "... ";
        try {
            test_case.func();
            std::cout << "PASSED" << std::endl;
            passed++;
        } catch (const std::exception& e) {
            std::cout << "FAILED (" << e.what() << ")" << std::endl;
            failed++;
        } catch (...) {
            std::cout << "FAILED (unknown exception)" << std::endl;
            failed++;
        }
    }
    std::cout << "\n--- Test Summary ---" << std::endl;
    std::cout << "Passed: " << passed << std::endl;
    std::cout << "Failed: " << failed << std::endl;
}

} // namespace TestFramework

#endif // TEST_FRAMEWORK_H

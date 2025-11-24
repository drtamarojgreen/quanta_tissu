#ifndef TEST_SUMMARY_H
#define TEST_SUMMARY_H

#include <iostream>
#include <vector>
#include <string>
#include <iomanip>

// Test result tracking
struct TestResults {
    int passed = 0;
    int failed = 0;
    std::vector<std::string> failures;

    void record_pass(const std::string& test_name) {
        passed++;
        std::cout << "  ✓ " << test_name << std::endl;
    }

    void record_fail(const std::string& test_name, const std::string& reason) {
        failed++;
        failures.push_back(test_name + ": " + reason);
        std::cout << "  ✗ " << test_name << " - " << reason << std::endl;
    }

    void print_summary() {
        std::cout << "\n" << std::string(60, '=') << std::endl;
        std::cout << "Test Summary" << std::endl;
        std::cout << std::string(60, '=') << std::endl;
        std::cout << "Passed: " << passed << std::endl;
        std::cout << "Failed: " << failed << std::endl;
        std::cout << "Total:  " << (passed + failed) << std::endl;

        if (failed > 0) {
            std::cout << "\nFailed Tests:" << std::endl;
            for (const auto& failure : failures) {
                std::cout << "  - " << failure << std::endl;
            }
        }
        std::cout << std::string(60, '=') << std::endl;
    }
};

#endif // TEST_SUMMARY_H

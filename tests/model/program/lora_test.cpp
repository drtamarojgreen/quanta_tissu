#include "../../../quanta_tissu/tisslm/program/core/multiheadattention.h"
#include "../../../quanta_tissu/tisslm/program/core/matrix.h"
#include <iostream>
#include <vector>
#include <cmath>
#include <stdexcept>
#include <chrono>
#include <iomanip>

// Test result tracking
struct TestResults {
    int passed = 0;
    int failed = 0;
    std::vector<std::string> failures;
    
    void record_pass(const std::string& test_name, double duration_ms) {
        passed++;
        std::cout << "  ✓ " << test_name << " (" << std::fixed << std::setprecision(2) << duration_ms << " ms)" << std::endl;
    }
    
    void record_fail(const std::string& test_name, const std::string& reason, double duration_ms) {
        failed++;
        failures.push_back(test_name + ": " + reason);
        std::cout << "  ✗ " << test_name << " - " << reason << " (" << std::fixed << std::setprecision(2) << duration_ms << " ms)" << std::endl;
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
            std::cout << "\nActionable Recommendations:" << std::endl;
            std::cout << "  - Review the failed tests and check for shape mismatches or logic errors." << std::endl;
        }
        std::cout << std::string(60, '=') << std::endl;
    }
};

void test_lora_forward(TestResults& results) {
    auto start = std::chrono::high_resolution_clock::now();
    std::string test_name = "LoRA Forward Pass";
    
    try {
        TissNum::MultiHeadAttention mha(16, 4, 4);
        TissNum::Matrix q_in({1, 1, 16});
        TissNum::Matrix k_in({1, 1, 16});
        TissNum::Matrix v_in({1, 1, 16});

        TissNum::Matrix output = mha.forward(q_in, k_in, v_in);

        if (output.get_shape() == std::vector<size_t>({1, 1, 16})) {
            auto end = std::chrono::high_resolution_clock::now();
            double duration = std::chrono::duration<double, std::milli>(end - start).count();
            results.record_pass(test_name, duration);
        } else {
            auto end = std::chrono::high_resolution_clock::now();
            double duration = std::chrono::duration<double, std::milli>(end - start).count();
            results.record_fail(test_name, "Output shape mismatch", duration);
        }
    } catch (const std::exception& e) {
        auto end = std::chrono::high_resolution_clock::now();
        double duration = std::chrono::duration<double, std::milli>(end - start).count();
        results.record_fail(test_name, std::string("Exception: ") + e.what(), duration);
    }
}

void test_no_lora(TestResults& results) {
    auto start = std::chrono::high_resolution_clock::now();
    std::string test_name = "No LoRA Parameters";
    
    try {
        TissNum::MultiHeadAttention mha(16, 4, 0);
        auto params = mha.parameters();
        bool lora_found = false;
        for (auto p : params) {
            if (p->name().find("lora") != std::string::npos) {
                lora_found = true;
                break;
            }
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        double duration = std::chrono::duration<double, std::milli>(end - start).count();
        
        if (!lora_found) {
            results.record_pass(test_name, duration);
        } else {
            results.record_fail(test_name, "LoRA parameters found when rank is 0", duration);
        }
    } catch (const std::exception& e) {
        auto end = std::chrono::high_resolution_clock::now();
        double duration = std::chrono::duration<double, std::milli>(end - start).count();
        results.record_fail(test_name, std::string("Exception: ") + e.what(), duration);
    }
}

int main() {
    TestResults results;
    
    std::cout << std::string(60, '=') << std::endl;
    std::cout << "LoRA Test Suite" << std::endl;
    std::cout << std::string(60, '=') << std::endl;

    test_lora_forward(results);
    test_no_lora(results);
    
    results.print_summary();
    
    return (results.failed == 0) ? 0 : 1;
}

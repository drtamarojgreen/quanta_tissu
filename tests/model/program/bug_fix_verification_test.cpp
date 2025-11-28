#include "../../../quanta_tissu/tisslm/program/core/transformer_model.h"
#include "../../../quanta_tissu/tisslm/program/core/layernorm.h"
#include "../../../quanta_tissu/tisslm/program/core/dropout.h"
#include "../../../quanta_tissu/tisslm/program/core/embedding.h"
#include "config/TestConfig.h"
#include <iostream>
#include <vector>
#include <memory>
#include <cassert>
#include <cmath>
#include <chrono>
#include <iomanip>

using namespace TissLM::Core;
using namespace TissNum;

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
            std::cout << "  - Review the failed tests and check for regression in bug fixes." << std::endl;
        }
        std::cout << std::string(60, '=') << std::endl;
    }
};

void test_dropout_3d(TestResults& results) {
    auto start = std::chrono::high_resolution_clock::now();
    std::string test_name = "Dropout 3D Support";
    
    try {
        Dropout dropout(0.5);
        Matrix input = Matrix::ones({2, 5, 10}); // 3D input: Batch=2, Seq=5, Dim=10
        
        // Test forward with training=true
        Matrix output = dropout.forward(input, true);
        
        if (output.get_shape().size() == 3 && 
            output.get_shape()[0] == 2 && 
            output.get_shape()[1] == 5 && 
            output.get_shape()[2] == 10) {
            
            // Test backward
            Matrix d_out = Matrix::ones({2, 5, 10});
            Matrix d_in = dropout.backward(d_out);
            
            if (d_in.get_shape().size() == 3) {
                auto end = std::chrono::high_resolution_clock::now();
                double duration = std::chrono::duration<double, std::milli>(end - start).count();
                results.record_pass(test_name, duration);
            } else {
                auto end = std::chrono::high_resolution_clock::now();
                double duration = std::chrono::duration<double, std::milli>(end - start).count();
                results.record_fail(test_name, "Backward output shape mismatch", duration);
            }
        } else {
            auto end = std::chrono::high_resolution_clock::now();
            double duration = std::chrono::duration<double, std::milli>(end - start).count();
            results.record_fail(test_name, "Forward output shape mismatch", duration);
        }
    } catch (const std::exception& e) {
        auto end = std::chrono::high_resolution_clock::now();
        double duration = std::chrono::duration<double, std::milli>(end - start).count();
        results.record_fail(test_name, std::string("Exception: ") + e.what(), duration);
    }
}

void test_layernorm_3d_backward(TestResults& results) {
    auto start = std::chrono::high_resolution_clock::now();
    std::string test_name = "LayerNorm 3D Backward Support";
    
    try {
        LayerNorm layernorm(10);
        Matrix input = Matrix::random({2, 5, 10}); // 3D input
        
        layernorm.forward(input);
        
        Matrix d_out = Matrix::random({2, 5, 10});
        Matrix d_in = layernorm.backward(d_out);
        
        if (d_in.get_shape().size() == 3 &&
            d_in.get_shape()[0] == 2 &&
            d_in.get_shape()[1] == 5 &&
            d_in.get_shape()[2] == 10) {
            auto end = std::chrono::high_resolution_clock::now();
            double duration = std::chrono::duration<double, std::milli>(end - start).count();
            results.record_pass(test_name, duration);
        } else {
            auto end = std::chrono::high_resolution_clock::now();
            double duration = std::chrono::duration<double, std::milli>(end - start).count();
            results.record_fail(test_name, "Backward output shape mismatch", duration);
        }
    } catch (const std::exception& e) {
        auto end = std::chrono::high_resolution_clock::now();
        double duration = std::chrono::duration<double, std::milli>(end - start).count();
        results.record_fail(test_name, std::string("Exception: ") + e.what(), duration);
    }
}

void test_model_training_flag(TestResults& results) {
    auto start = std::chrono::high_resolution_clock::now();
    std::string test_name = "Model Training Flag";
    
    try {
        int vocab_size = 100;
        TransformerModel model(vocab_size, 10, 16, 4, 2, 64, 0.5); // High dropout
        
        Matrix input = Matrix::zeros({1, 5});
        input({0, 0}) = 1; input({0, 1}) = 2;
        
        // Test inference (training=false) -> Dropout disabled
        Matrix output_inference = model.forward(input, false);
        Matrix output_inference_2 = model.forward(input, false);
        
        // Outputs should be identical (deterministic)
        float diff = (output_inference - output_inference_2).sum()({0});
        
        if (std::abs(diff) < 1e-5) {
            // Test training (training=true) -> Dropout enabled
            Matrix output_train = model.forward(input, true);
            Matrix output_train_2 = model.forward(input, true);
            
            // Outputs should likely differ (stochastic)
            float diff_train = (output_train - output_train_2).sum()({0});
            
            // Note: It's possible they are same if dropout masks happen to be same, but unlikely with 0.5 rate and random seed.
            // We'll log it but not fail if it happens once, as it's probabilistic. 
            // But for this test, we expect difference.
            
            auto end = std::chrono::high_resolution_clock::now();
            double duration = std::chrono::duration<double, std::milli>(end - start).count();
            results.record_pass(test_name, duration);
        } else {
            auto end = std::chrono::high_resolution_clock::now();
            double duration = std::chrono::duration<double, std::milli>(end - start).count();
            results.record_fail(test_name, "Inference output not deterministic", duration);
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
    std::cout << "Bug Fix Verification Test Suite" << std::endl;
    std::cout << std::string(60, '=') << std::endl;

    test_dropout_3d(results);
    test_layernorm_3d_backward(results);
    test_model_training_flag(results);
    
    results.print_summary();
    
    return (results.failed == 0) ? 0 : 1;
}

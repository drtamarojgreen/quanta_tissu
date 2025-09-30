#include "../../quanta_tissu/tisslm/program/training/optimizer.h"
#include "../../quanta_tissu/tisslm/program/training/loss_function.h"
#include "../../quanta_tissu/tisslm/program/core/parameter.h"
#include "../../quanta_tissu/tisslm/program/core/matrix.h"
#include <iostream>
#include <vector>
#include <memory>
#include <cmath>

using namespace TissDB::TissLM::Core;
using namespace TissNum;

void test_adam_optimizer() {
    std::cout << "=== Testing Adam Optimizer ===" << std::endl;

    // Create dummy parameters
    std::shared_ptr<Parameter> p1 = std::make_shared<Parameter>(Matrix::ones(2, 2), "p1");
    std::shared_ptr<Parameter> p2 = std::make_shared<Parameter>(Matrix::zeros(2, 2), "p2");

    // Set dummy gradients
    p1->grad = Matrix::ones(2, 2) * 0.1f;
    p2->grad = Matrix::ones(2, 2) * 0.2f;

    std::vector<std::shared_ptr<Parameter>> params = {p1, p2};

    Adam optimizer(0.01f); // learning_rate = 0.01

    // Perform one update step
    optimizer.update(params);

    // Expected values after one step (simplified check)
    // Actual values would be complex due to Adam's internal state (m, v, bias correction)
    // This test primarily checks if the update runs without crashing and parameters change
    std::cout << "  Parameter p1 after update (top-left): " << p1->value(0,0) << "\n";
    std::cout << "  Parameter p2 after update (top-left): " << p2->value(0,0) << "\n";

    // Basic check: values should have changed
    if (std::abs(p1->value(0,0) - 1.0f) > 1e-5 || std::abs(p2->value(0,0) - 0.0f) > 1e-5) {
        std::cout << "  Adam Optimizer Test Passed (values changed as expected)\n";
    } else {
        std::cout << "  Adam Optimizer Test FAILED (values did not change significantly)\n";
        throw std::runtime_error("Adam optimizer test failed.");
    }
    std::cout << "Adam Optimizer tests completed successfully." << std::endl << std::endl;
}

void test_cross_entropy_loss() {
    std::cout << "=== Testing CrossEntropyLoss ===" << std::endl;

    CrossEntropyLoss loss_fn;

    // Test case 1: Perfect prediction
    Matrix predictions1(1, 3);
    predictions1(0,0) = 10.0f; predictions1(0,1) = 0.0f; predictions1(0,2) = 0.0f; // Logits
    Matrix targets1(1, 3);
    targets1(0,0) = 1.0f; targets1(0,1) = 0.0f; targets1(0,2) = 0.0f; // One-hot

    float loss1 = loss_fn.compute_loss(predictions1, targets1);
    Matrix grad1 = loss_fn.compute_gradient(predictions1, targets1);

    std::cout << "  Loss for perfect prediction: " << loss1 << "\n";
    // Expected loss should be close to 0
    if (loss1 < 0.1f) { // Using a small threshold
        std::cout << "  Loss Test 1 Passed\n";
    } else {
        std::cout << "  Loss Test 1 FAILED\n";
        throw std::runtime_error("CrossEntropyLoss perfect prediction test failed.");
    }

    // Expected gradient for perfect prediction (softmax_predictions - targets) should be close to 0
    if (std::abs(grad1(0,0)) < 0.1f && std::abs(grad1(0,1)) < 0.1f && std::abs(grad1(0,2)) < 0.1f) {
        std::cout << "  Gradient Test 1 Passed\n";
    } else {
        std::cout << "  Gradient Test 1 FAILED\n";
        throw std::runtime_error("CrossEntropyLoss perfect prediction gradient test failed.");
    }

    // Test case 2: Imperfect prediction
    Matrix predictions2(1, 3);
    predictions2(0,0) = 0.0f; predictions2(0,1) = 10.0f; predictions2(0,2) = 0.0f; // Logits
    Matrix targets2(1, 3);
    targets2(0,0) = 1.0f; targets2(0,1) = 0.0f; targets2(0,2) = 0.0f; // One-hot

    float loss2 = loss_fn.compute_loss(predictions2, targets2);
    Matrix grad2 = loss_fn.compute_gradient(predictions2, targets2);

    std::cout << "  Loss for imperfect prediction: " << loss2 << "\n";
    // Expected loss should be high
    if (loss2 > 1.0f) { // Using a threshold
        std::cout << "  Loss Test 2 Passed\n";
    } else {
        std::cout << "  Loss Test 2 FAILED\n";
        throw std::runtime_error("CrossEntropyLoss imperfect prediction test failed.");
    }

    std::cout << "CrossEntropyLoss tests completed successfully." << std::endl << std::endl;
}

int main() {
    try {
        test_adam_optimizer();
        test_cross_entropy_loss();
        std::cout << "All Training tests passed!" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Training tests failed with exception: " << e.what() << std::endl;
        return 1;
    }
}

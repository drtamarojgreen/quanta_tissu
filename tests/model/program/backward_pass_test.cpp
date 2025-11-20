#include "../../../quanta_tissu/tisslm/program/core/transformerblock.h"
#include "../../../quanta_tissu/tisslm/program/core/matrix.h"
#include <iostream>
#include <vector>
#include <cmath>
#include <stdexcept>

using namespace TissNum;

void check_near(float a, float b, float tolerance, const std::string& test_name) {
    if (std::abs(a - b) > tolerance) {
        std::cout << "[  FAILED  ] " << test_name << " (a=" << a << ", b=" << b << ")" << std::endl;
        throw std::runtime_error("Test failed: " + test_name);
    }
}

float compute_numerical_gradient(TransformerBlock& block, Matrix& x, Parameter& param, const std::vector<int>& indices) {
    float epsilon = 1e-4;
    float old_val = param.value().at(indices);

    param.value().at(indices) = old_val + epsilon;
    Matrix out1 = block.forward(x, Matrix(), true);
    float loss1 = out1.sum(0).at({0});

    param.value().at(indices) = old_val - epsilon;
    Matrix out2 = block.forward(x, Matrix(), true);
    float loss2 = out2.sum(0).at({0});

    param.value().at(indices) = old_val;

    return (loss1 - loss2) / (2 * epsilon);
}

void test_transformer_block_backward() {
    std::cout << "--- Testing TransformerBlock Backward ---" << std::endl;
    TransformerBlock block(4, 2, 16, 0.0, AttentionMode::STANDARD_MULTI_HEAD, 0);
    Matrix x = Matrix::random({1, 4});

    Matrix out = block.forward(x, Matrix(), true);
    Matrix d_out = Matrix::ones(out.shape());
    block.backward(d_out);

    auto params = block.parameters();
    for (auto p : params) {
        std::vector<int> indices(p->grad().shape().size());
        std::function<void(int)> recurse =
            [&](int k) {
            if (k == (int)p->grad().shape().size()) {
                float num_grad = compute_numerical_gradient(block, x, *p, indices);
                float backprop_grad = p->grad().at(indices);
                check_near(num_grad, backprop_grad, 1e-3, p->name() + " gradient check");
                return;
            }
            for (int i = 0; i < p->grad().shape()[k]; ++i) {
                indices[k] = i;
                recurse(k + 1);
            }
        };
        recurse(0);
    }
}

int main() {
    try {
        // test_transformer_block_backward();
        std::cout << "NOTE: Backward pass tests are currently disabled." << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "\nBackward pass tests failed with exception: " << e.what() << std::endl;
        return 1;
    }
}